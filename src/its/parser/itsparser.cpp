/*  This file is part of LoAT.
 *  Copyright (c) 2015-2016 Matthias Naaf, RWTH Aachen University, Germany
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses>.
 */

#include "itsparser.h"

#include <fstream>
#include <boost/algorithm/string.hpp>

#include "termparser.h"
#include "its/itsproblem.h"


using namespace parser;

using namespace std;
using namespace boost::algorithm;
using boost::optional;


ITSProblem ITSParser::loadFromFile(const string &filename) {
    ITSParser parser;
    return parser.load(filename);
}


ITSProblem ITSParser::load(const std::string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw FileError("Unable to open file: "+filename);
    }

    parseFile(file);
    convertRules();
    return itsProblem;
}

/**
 * Replaces symbols that ginac does not allow by underscores _
 * @param name the variable name to be modified
 */
std::string ITSParser::escapeVariableName(const std::string &name) {
    assert(!name.empty());

    std::string escapedName = name;
    for (int i = 0; i < escapedName.length(); ++i) {
        //replace I to avoid interpretation as complex number
        if (escapedName[i] == 'I') {
            escapedName[i] = 'Q';
        }

        //escape any symbol ginac can probably not parse
        if (!isalnum(escapedName[i])) {
            escapedName[i] = '_';
        }
    }

    //ensure name starts with a letter
    if (!isalpha(escapedName[0])) {
        escapedName = "v" + escapedName;
    }

    return escapedName;
}

/**
 * Initial parsing function, parses the given file into the intermediate representation (ParsedRule)
 */
void ITSParser::parseFile(ifstream &file) {
    bool has_vars, has_goal, has_start;
    has_vars = has_goal = has_start = false;

    bool in_rules = false;

    string line;
    while (getline(file, line)) {
        trim(line);
        if (line.length() == 0 || line[0] == '#') continue; // skip empty/comments

        if (in_rules) {
            // parseRule requires the termParser to be initialized
            assert(termParser);

            if (line == ")")
                in_rules = false;
            else {
                ParsedRule rule = parseRule(line);
                parsedRules.push_back(rule);
            }

        } else {
            if (line[0] != '(') throw FileError("Malformed line: "+line);

            if (line == "(RULES") {
                if (!has_goal || !has_vars || !has_start) {
                    throw FileError("Missing declarations (GOAL/STARTTERM/VAR) before RULES-block");
                }

                // At this point, we know all variables and can thus initialize the term parser
                termParser = std::unique_ptr<TermParser>(new TermParser(knownVariables, Config::Parser::AllowDivision));
                in_rules = true;

            } else if (line.back() != ')') {
                throw FileError("Malformed line (missing closing paren): "+line);

            } else if (line == "(GOAL COMPLEXITY)") {
                has_goal = true;

            } else if (line.substr(1,9) == "STARTTERM") {
                if (has_start) throw FileError("Multiple STARTTERM declarations");

                if (line.find("CONSTRUCTOR-BASED") != string::npos) {
                    // Support invalid format for benchmark, assume first rule defines start symbol.
                    // Note that ITSProblem's initialLocation defaults to 0. Since we process the
                    // first rule's lhs first, 0 will be mapped to the first rule's lhs location.
                    initialLocation.clear();

                } else {
                    string keyword = "FUNCTIONSYMBOLS ";
                    auto pos = line.find(keyword);
                    if (pos == string::npos) throw FileError("Invalid start term declaration: "+line);
                    pos += keyword.length();

                    auto endpos = line.find(')',pos);
                    if (endpos == string::npos) throw FileError("Missing ) in term declaration: "+line);
                    initialLocation = line.substr(pos,endpos-pos);
                }
                debugParser("Found start location: " << initialLocation);
                has_start = true;
            }

            else if (line.substr(1,3) == "VAR") {
                if (has_vars) throw FileError("Multiple VAR declarations");

                stringstream ss(line.substr(4,line.length()-1-4));
                string varname;
                vector<string> varnames;

                while (ss >> varname) {
                    varnames.push_back(varname);
                }

                // sort the variable names to make sure that the result of the analysis does not depend
                // on the order in which the variables were declared (which happened in the past)
                std::sort(varnames.begin(), varnames.end());
                for (string varname: varnames) {
                    VariableIdx var = itsProblem.addFreshVariable(escapeVariableName(varname));
                    knownVariables.emplace(varname, var);
                }

                debugParser("Found variable declaration with " << knownVariables.size() << " entries");
                has_vars = true;
            }

            else {
                throw FileError("Unexpected line: "+line);
            }
        }
    }

    // Ensure we have at least some rules
    if (parsedRules.empty()) throw FileError("No rules defined");

    // Issue a warning if the initial location is not explicitly defined
    if (initialLocation.empty()) {
        debugParser("WARNING: Missing start term, defaulting to first function symbol");
    }
}



// #########################################
// ##  Step 1: Parse rules as ParsedRule  ##
// #########################################

/**
 * Parses a rule in the ITS file format reading from line.
 * @param res the current state of the ITS that is read (read and modified)
 * @param knownVariables mapping from string to the corresponding variable index (read and modified)
 * @param line the input string
 */
ITSParser::ParsedRule ITSParser::parseRule(const string &line) const {
    debugParser("parsing rule: " << line);

    /* split string into lhs, rhs (and possibly cost in between) */
    string lhs,rhs,cost;
    string::size_type pos = line.find("-{");
    if (pos != string::npos) {
        //-{ cost }> sytnax
        auto endpos = line.find("}>");
        if (endpos == string::npos) throw ITSParser::FileError("Invalid rule, malformed -{ cost }>: "+line);
        cost = line.substr(pos+2,endpos-(pos+2));
        lhs = line.substr(0,pos);
        rhs = line.substr(endpos+2);
    } else {
        //default -> syntax (leave cost string empty)
        pos = line.find("->");
        if (pos == string::npos) throw ITSParser::FileError("Invalid rule, -> missing: "+line);
        lhs = line.substr(0,pos);
        rhs = line.substr(pos+2);
    }
    trim(lhs);

    /* split rhs into rhs funapp and guard */
    string guard;
    if ((pos = rhs.find("[")) != string::npos) {
        if (rhs.back() != ']') throw ITSParser::FileError("Invalid rule, ']' missing: "+line);
        guard = rhs.substr(pos+1,rhs.length()-1-(pos+1));
        trim(guard);
        rhs = rhs.substr(0,pos);
    } else if ((pos = rhs.find(":|:")) != string::npos) {
        guard = rhs.substr(pos+3);
        trim(guard);
        rhs = rhs.substr(0,pos);
    }
    trim(rhs);

    /* parse all individual parts */
    ParsedRule res;
    res.lhs = parseLeftHandSide(lhs);
    res.rhss = parseRightHandSide(rhs);
    res.cost = parseCost(cost);
    res.guard = parseGuard(guard);
    return res;
}

TermPtr ITSParser::parseTerm(const std::string &s) const {
    try {
        return termParser->parseTerm(s);
    } catch (TermParser::TermParserException e) {
        throw FileError("Error while parsing term '" + s + "': " + e.what());
    }
}

TermPtr ITSParser::parseLeftHandSide(const std::string &lhs) const {
    TermPtr res = parseTerm(lhs);

    if (!res->isFunappOnArithmeticExpressions()) {
        throw FileError("Invalid left-hand side: "+lhs);
    }

    // check that all arguments are variables and no variable occurs twice
    set<VariableIdx> vars;
    const TermFunApp *funapp = static_cast<TermFunApp*>(res.get());

    for (TermPtr arg : funapp->getArguments()) {
        if (arg->getType() != Term::Variable) {
            throw FileError("Invalid left-hand side, argument is not a variable: "+lhs);
        }

        VariableIdx var = static_cast<TermVariable*>(arg.get())->getVariableIdx();
        if (vars.count(var) != 0) {
            throw FileError("Invalid left-hand side, variables are not distinct: "+lhs);
        }
        vars.insert(var);
    }

    return res;
}


vector<TermPtr> ITSParser::parseRightHandSide(const std::string &rhs) const {
    vector<TermPtr> res;
    TermPtr rhsTerm = parseTerm(rhs);

    if (rhsTerm->getType() != Term::FunctionApplication) {
        throw FileError("Invalid right-hand side, not a function application: "+rhs);
    }

    const TermFunApp *funapp = static_cast<TermFunApp*>(rhsTerm.get());
    string name = funapp->getName();

    // Handle Com_n syntax for multiple calls
    if (name.substr(0,4) == "Com_") {
        int n = std::stoi(name.substr(4));
        if (n != funapp->getArguments().size()) {
            throw FileError("Com_n application has incorrect number of arguments: "+rhs);
        }
        res = funapp->getArguments();

    } else {
        res = { rhsTerm }; //singleton list
    }

    // Check that all rhs terms are well-formed
    for (int i=0; i < res.size(); ++i) {
        if (!res[i]->isFunappOnArithmeticExpressions()) {
            throw FileError("Invalid rhs, argument "+std::to_string(i)+" is malformed: "+rhs);
        }
    }
    return res;
}


optional<TermPtr> ITSParser::parseCost(const std::string &cost) const {
    if (cost.empty()) {
        return optional<TermPtr>();
    } else {
        return optional<TermPtr>(parseTerm(cost));
    }
}


vector<Relation> ITSParser::parseGuard(const std::string &guard) const {
    // symbols and corresponding operators (order is important to avoid parsing == as =)
    const std::string symbols[7] = { "==", "!=", ">=", "<=", ">", "<", "=" };
    const Relation::Operator ops[7] = {
        Relation::RelationEqual,
        Relation::RelationNotEqual,
        Relation::RelationGreaterEqual,
        Relation::RelationLessEqual,
        Relation::RelationGreater,
        Relation::RelationLess,
        Relation::RelationEqual
    };

    vector<Relation> res;
    string::size_type pos;

    if (!guard.empty()) {
        string::size_type startpos = 0;
        do {
            // read until the next occurrence of /\ or && (or until the end of line)
            pos = min(guard.find("/\\",startpos),guard.find("&&",startpos));
            string term = guard.substr(startpos,(pos == string::npos) ? string::npos : (pos-startpos));
            trim(term);

            // update startpos to point right behind /\ or &&
            startpos = pos+2;

            //ignore TRUE in guards (used to indicate an empty guard in some files)
            if (term == "TRUE" || term.empty()) continue;

            string::size_type relpos;
            std::string lhs, rhs;
            for (int i=0; i < 7; ++i) {
                relpos = term.find(symbols[i]);
                if (relpos != string::npos) {
                    lhs = term.substr(0, relpos);
                    trim(lhs);

                    rhs = term.substr(relpos + symbols[i].size());
                    trim(rhs);

                    TermPtr l = parseTerm(lhs);
                    TermPtr r = parseTerm(rhs);

                    res.push_back(Relation(l, r, ops[i]));
                    goto next;
                }
            }

            throw FileError("Can't parse guard, missing relational operator: "+guard);
            next:;

        } while (pos != string::npos);
    }

    return res;
}



// #####################################
// ##  Step 2: Convert to ITSProblem  ##
// #####################################

void ITSParser::convertRules() {
    assert(knownLocations.empty());

    // First create all locations and map their names to their indices.
    // While doing so, we check that all occurrences of a function symbol have the same arity.
    // Note that we consider all left-hand sides before considering right-hand sides.
    for (const ParsedRule &rule : parsedRules) {
        addAndCheckLocationData(rule.lhs, true);
    }
    for (const ParsedRule &rule : parsedRules) {
        for (TermPtr rhs : rule.rhss) {
            addAndCheckLocationData(rhs, false);
        }
    }

    // Convert all rules to the more convenient representation in ITSProblem.
    //
    // The crucial step is the abstraction from rhs terms to updates
    // (e.g. "f(x) -> g(x+2)" could result in the update "y := x+2". Note the "y" -- we use the variable
    //  that is used in the lhs of g. So if there is a rule like "g(y) -> ...", we identify y with the term x+2).
    //
    // To this end, we make sure that all rules with the same lhs function symbol use the same variables on the lhs
    // (e.g. for "f(x) -> ...", "f(y) -> ..." we rename the variables such that both rules are "f(x) -> ...").
    // We also make sure that unbounded variables (that don't occur on the lhs) are replaced by temporary variables.
    for (const ParsedRule &rule : parsedRules) {
        addParsedRule(rule);
    }

    // Set the start location, if given
    if (!initialLocation.empty()) {
        auto it = knownLocations.find(initialLocation);
        if (it == knownLocations.end()) {
            throw FileError("Declared startterm does not exist: "+initialLocation);
        }
        itsProblem.setInitialLocation(it->second.index);
    }
}


void ITSParser::addParsedRule(const ParsedRule &rule) {
    // Convert lhs to Ginac expressions
    LocationIdx lhsLoc = getLocationData(rule.lhs).index;
    Expression cost = rule.cost ? rule.cost.get()->toGinacExpression(itsProblem) : Expression(1);
    RuleLhs lhs(lhsLoc, {}, cost);

    if (!cost.is_polynomial(itsProblem.getGinacVarList())) {
        throw FileError("Non-polynomial cost in the input");
    }

    for (const Relation &rel : rule.guard) {
        lhs.getGuardMut().push_back(rel.toGinacExpression(itsProblem));
    }

    // Convert rhs, compute update
    vector<RuleRhs> rhss;
    for (TermPtr rhs : rule.rhss) {
        LocationData loc = getLocationData(rhs);
        const vector<TermPtr> args = static_cast<TermFunApp*>(rhs.get())->getArguments();

        LocationIdx rhsLoc = loc.index;
        UpdateMap rhsUpdate;
        for (int i=0; i < loc.arity; ++i) {
            VariableIdx var = loc.lhsVars[i];
            Expression update = args[i]->toGinacExpression(itsProblem);
            rhsUpdate.emplace(var, std::move(update));
        }

        rhss.push_back(RuleRhs(rhsLoc, rhsUpdate));
    }

    Rule newRule(std::move(lhs), std::move(rhss));

    // Ensure that a function symbol always occurs with the same lhs arguments,
    // e.g. if we have "f(x) -> ..." and "f(y) -> ..." we rename the variables in the second rule to get "f(x) -> ..."
    GiNaC::exmap subsLhs = computeSubstitutionToUnifyLhs(rule);
    newRule.applySubstitution(subsLhs);

    // Replace unbounded variables (which do not occur in lhs) by fresh temporary variables
    replaceUnboundedByTemporaryVariables(newRule, getLocationData(rule.lhs));

    // Remove trivial updates like "x := x" (to simplify rules)
    for (int i=0; i < newRule.rhsCount(); ++i) {
        stripTrivialUpdates(newRule.getUpdateMut(i));
    }

    itsProblem.addRule(newRule);
}


void ITSParser::addAndCheckLocationData(TermPtr term, bool lhs) {
    assert(term->getType() == Term::FunctionApplication);
    const TermFunApp *funapp = static_cast<TermFunApp*>(term.get());

    auto it = knownLocations.find(funapp->getName());
    if (it == knownLocations.end()) {
        // Create a new location
        LocationIdx idx = itsProblem.addNamedLocation(funapp->getName());

        // Gather relevant data about the new location
        LocationData loc;
        loc.index = idx;
        loc.arity = funapp->getArity();

        if (lhs) {
            for (TermPtr arg : funapp->getArguments()) {
                loc.lhsVars.push_back(static_cast<TermVariable*>(arg.get())->getVariableIdx());
            }
        } else {
            // Since we add all lhs locations before adding any rhs location,
            // this case only occurs if a location occurs only on rhs (never on any lhs).
            // We still have to set lhsVars, since they are needed for computing the update.
            // But since there is no lhs for this location, we just use arbitrary variables.
            for (int i=0; i < loc.arity; ++i) {
                if (itsProblem.hasVarIdx(i)) {
                    loc.lhsVars.push_back(i);
                } else {
                    loc.lhsVars.push_back(itsProblem.addFreshVariable("dummy"));
                }
            }
        }

        it = knownLocations.emplace(funapp->getName(), std::move(loc)).first;

    } else if (funapp->getArity() != it->second.arity) {
        throw FileError("Function symbol redeclared with different arity: "+funapp->getName());
    }
}


const ITSParser::LocationData& ITSParser::getLocationData(TermPtr term) const {
    assert(term->getType() == Term::FunctionApplication);
    const TermFunApp *funapp = static_cast<TermFunApp*>(term.get());

    auto it = knownLocations.find(funapp->getName());
    assert(it != knownLocations.end());
    return it->second;
}


set<VariableIdx> ITSParser::getVariables(const ParsedRule &rule) {
    set<VariableIdx> vars;
    rule.lhs->collectVariables(vars);
    if (rule.cost) {
        rule.cost.get()->collectVariables(vars);
    }
    for (const TermPtr &rhs : rule.rhss) {
        rhs->collectVariables(vars);
    }
    for (const Relation &rel : rule.guard) {
        rel.getLhs()->collectVariables(vars);
        rel.getRhs()->collectVariables(vars);
    }
    return vars;
}


GiNaC::exmap ITSParser::computeSubstitutionToUnifyLhs(const ParsedRule &rule) {
    GiNaC::exmap subs;
    const LocationData &loc = getLocationData(rule.lhs);
    const TermFunApp *funapp = static_cast<TermFunApp*>(rule.lhs.get());

    // Gather variables from rule
    set<VariableIdx> ruleVars = getVariables(rule);
    vector<VariableIdx> lhsVars;
    for (TermPtr arg : funapp->getArguments()) {
        lhsVars.push_back(static_cast<TermVariable*>(arg.get())->getVariableIdx());
    }

    // Replace variables in this rule such that lhsVars matches loc.lhsVars
    for (int i=0; i < loc.arity; ++i) {
        if (lhsVars[i] != loc.lhsVars[i]) {
            // Add substitution
            ExprSymbol oldSym = itsProblem.getGinacSymbol(lhsVars[i]);
            ExprSymbol newSym = itsProblem.getGinacSymbol(loc.lhsVars[i]);
            subs[oldSym] = newSym;
        }
    }

    // Make sure that different variables never coincide after applying the substitution.
    // e.g. if we replace "x/y" in "f(x) -> g(y)", the result "f(y) -> g(y)" is incorrect! Instead rename "y".
    GiNaC::exmap subsMore;
    for (const auto &it : subs) {
        ExprSymbol newSym = GiNaC::ex_to<GiNaC::symbol>(it.second);

        // If newSym is already replaced, everything is fine
        if (subs.count(newSym) > 0 || subsMore.count(newSym) > 0) continue;

        // Otherwise, if newSym occurs in rule, add it to the substitution
        VariableIdx newVar = itsProblem.getVarIdx(newSym);
        if (ruleVars.count(newVar) > 0) {
            VariableIdx freshVar = itsProblem.addFreshVariable(newSym.get_name());
            ExprSymbol freshSym = itsProblem.getGinacSymbol(freshVar);
            subsMore[newSym] = freshSym;
        }
    }

    // Combine subs and subsMore (such that they are executed in parallel, do *not* compose them)
    for (const auto &it : subsMore) {
        assert(subs.count(it.first) == 0);
        subs[it.first] = it.second;
    }

    return subs;
}


void ITSParser::replaceUnboundedByTemporaryVariables(Rule &rule, const LocationData &lhsData) {
    // Gather variables
    ExprSymbolSet ruleVars = getSymbols(rule);

    ExprSymbolSet lhsVars;
    for (VariableIdx var : lhsData.lhsVars) {
        lhsVars.insert(itsProblem.getGinacSymbol(var));
    }

    // Substitute all variables that do not occur on the lhs by temporary ones
    GiNaC::exmap subs;
    for (ExprSymbol var : ruleVars) {
        if (lhsVars.count(var) == 0) {
            // Create a fresh temporary variable
            VariableIdx tv = itsProblem.addFreshTemporaryVariable("free");
            subs[var] = itsProblem.getGinacSymbol(tv);
        }
    }

    rule.applySubstitution(subs);
}


ExprSymbolSet ITSParser::getSymbols(const Rule &rule) {
    ExprSymbolSet res;

    // lhs
    rule.getCost().collectVariables(res);
    for (const Expression &ex : rule.getGuard()) {
        ex.collectVariables(res);
    }

    // rhs
    // Note: For an update like x/y, only y is counted, since x is not part of this rule (but of a different lhs)
    for (auto rhs = rule.rhsBegin(); rhs != rule.rhsEnd(); ++rhs) {
        for (const auto &it : rhs->getUpdate()) {
            it.second.collectVariables(res);
        }
    }

    return res;
}


void ITSParser::stripTrivialUpdates(UpdateMap &update) const {
    set<VariableIdx> toRemove;

    for (const auto &it : update) {
        ExprSymbol lhs = itsProblem.getGinacSymbol(it.first);
        const Expression &rhs = it.second;

        if (rhs.equalsVariable(lhs)) {
            toRemove.insert(it.first);
        }
    }

    for (VariableIdx var : toRemove) {
        update.erase(var);
    }
}




















