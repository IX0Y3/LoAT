#include "t2parser.hpp"
#include "../types.hpp"
#include "../rule.hpp"

#include <fstream>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>

namespace t2parser {

    ITSProblem T2Parser::loadFromFile(const std::string &filename) {
        T2Parser parser;
        parser.run(filename);
        return parser.res;
    }

    void T2Parser::run(const std::string &filename) {
        std::ifstream ifs(filename);
        std::string line;
        while (getline(ifs, line)) {
            line = removeComment(line);
            if (boost::starts_with(line, START)) {
                assert(locs.empty());
                LocationIdx loc = getLoc(trim(line, START, ";"));
                res.setInitialLocation(loc);
            } else if (boost::starts_with(line, FROM)) {
                parseTransition(getLoc(trim(line, FROM, ";")), ifs);
            }
        }
    }

    void T2Parser::parseTransition(LocationIdx start, std::ifstream &ifs) {
        std::string line;
        GuardList guard;
        UpdateMap update;
        while (getline(ifs, line)) {
            line = removeComment(line);
            if (boost::starts_with(line, TO)) {
                res.addRule(Rule(start, guard, Expression(1), getLoc(trim(line, TO, ";")), update));
                return;
            } else if (boost::starts_with(line, ASSUME)) {
                guard.push_back(parseConstraint(trim(line, ASSUME, ");")));
            } else {
                boost::algorithm::trim(line);
                if (line.empty()) {
                    continue;
                }
                int pos = line.find(ASSIGN);
                std::string lhs = line.substr(0, pos);
                boost::algorithm::trim(lhs);
                addVar(lhs);
                std::string rhs = line.substr(pos + ASSIGN.size(), line.size() - pos - ASSIGN.size() - 1);
                update[vars[lhs]] = parseExpression(rhs);
            }
        }
    }

    std::string T2Parser::removeComment(const std::string &string) {
        if (string.find("//") == std::string::npos) {
            return string;
        } else {
            return string.substr(0, string.find("//"));
        }
    }

    Expression T2Parser::parseConstraint(const std::string &str) {
        if (str.find(">=") != std::string::npos) {
            unsigned int pos = str.find(">=");
            Expression lhs = parseExpression(str.substr(0, pos));
            Expression rhs = parseExpression(str.substr(pos + 2, str.size() - 1));
            return lhs >= rhs;
        } else if (str.find("<=") != std::string::npos) {
            unsigned int pos = str.find("<=");
            Expression lhs = parseExpression(str.substr(0, pos));
            Expression rhs = parseExpression(str.substr(pos + 2, str.size() - 1));
            return lhs <= rhs;
        } else if (str.find("==") != std::string::npos) {
            unsigned int pos = str.find("==");
            Expression lhs = parseExpression(str.substr(0, pos));
            Expression rhs = parseExpression(str.substr(pos + 2, str.size() - 1));
            return lhs == rhs;
        } else if (str.find("<") != std::string::npos) {
            unsigned int pos = str.find("<");
            Expression lhs = parseExpression(str.substr(0, pos));
            Expression rhs = parseExpression(str.substr(pos + 2, str.size() - 1));
            return lhs < rhs;
        } else if (str.find(">") != std::string::npos) {
            unsigned int pos = str.find(">");
            Expression lhs = parseExpression(str.substr(0, pos));
            Expression rhs = parseExpression(str.substr(pos + 2, str.size() - 1));
            return lhs > rhs;
        } else {
            assert(false);
        }
    }

    Expression T2Parser::parseExpression(std::string str) {
        boost::algorithm::trim(str);
        if (str == NONDET) {
            return res.getVarSymbol(res.addFreshTemporaryVariable("nondet"));
        }
        GiNaC::parser reader(symtab);
        const GiNaC::ex &ex = reader(str);
        GiNaC::symtab table = reader.get_syms();
        for (const auto &e: table) {
            addVar(e.first);
        }
        return ex;
    }

    std::string T2Parser::trim(std::string toTrim, std::string prefix, std::string suffix) {
        boost::algorithm::trim(toTrim);
        assert(boost::starts_with(toTrim, prefix));
        assert(boost::ends_with(toTrim, suffix));
        std::string res(toTrim.substr(prefix.size(), toTrim.size() - prefix.size() - suffix.size()));
        boost::algorithm::trim(res);
        return res;
    }

    VariableIdx T2Parser::addVar(const std::string &str) {
        if (vars.find(str) != vars.end()) {
            return vars[str];
        }
        VariableIdx var = res.addFreshVariable(str);
        vars[str] = var;
        symtab[str] = res.getVarSymbol(var);
        return var;
    }

    LocationIdx T2Parser::getLoc(const std::string &str) {
        if (locs.find(str) != locs.end()) {
            return locs[str];
        }
        LocationIdx loc = res.addLocation();
        locs[str] = loc;
        return loc;
    }

}
