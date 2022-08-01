/*  This file is part of LoAT.
 *  Copyright (c) 2018-2019 Florian Frohn
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

#include "loopacceleration.hpp"

#include "../smt/smt.hpp"
#include "../smt/smtfactory.hpp"
#include "../asymptotic/asymptoticbound.hpp"
#include "recurrence/recurrence.hpp"
#include "meter/metertools.hpp"
#include "../expr/guardtoolbox.hpp"

#include <purrs.hh>
#include "../util/relevantvariables.hpp"
#include "../analysis/chain.hpp"
#include "accelerationCalculus/accelerationproblem.hpp"
#include "iterationCounterElimination/vareliminator.hpp"
#include "../util/status.hpp"
#include "../its/export.hpp"

using namespace std;

LoopAcceleration::LoopAcceleration(ITSProblem &its, const LinearRule &rule, LocationIdx sink, Complexity cpx)
        : its(its), rule(rule), sink(sink), cpx(cpx) {}

bool LoopAcceleration::shouldAccelerate() const {
    return !rule.getCost().isNontermSymbol() && (Config::Analysis::nonTermination() || rule.getCost().isPoly());
}

vector<Rule> LoopAcceleration::replaceByUpperbounds(const Var &N, const Rule &rule) {
    // gather all upper bounds (if possible)
    VarEliminator ve(rule.getGuard(), N, its);

    // avoid rule explosion (by not instantiating N if there are too many bounds)
    if (ve.getRes().empty() || ve.getRes().size() > Config::LoopAccel::MaxUpperboundsForPropagation) {
        return {};
    }

    // create one rule for each upper bound, by instantiating N with this bound
    vector<Rule> res;
    for (const Subs &subs : ve.getRes()) {
        if (subs.get(N).isRationalConstant()) continue;
        res.push_back(rule.subs(subs));
    }
    return res;
}

LinearRule LoopAcceleration::buildNontermRule(const BoolExpr guard) const {
    return LinearRule(rule.getLhsLoc(), guard, Expr::NontermSymbol, sink, {});
}

Acceleration::Result LoopAcceleration::run() {
    Acceleration::Result res;
    res.status = Failure;
    if (shouldAccelerate()) {
        option<AccelerationProblem> ap = AccelerationProblem::init(rule, its);
        if (ap) {
            std::vector<AccelerationProblem::Result> ars = ap->computeRes();
            for (const AccelerationProblem::Result &ar: ars) {
                unsigned vb = ap->getValidityBound();
                res.status = vb > 1 ? PartialSuccess : Success;
                if (ar.witnessesNonterm) {
                    option<Rule> resultingRule;
                    if (vb > 0) {
                        option<Rule> prefix = rule;
                        for (unsigned i = 0; i < vb - 1; ++i) {
                            prefix = Chaining::chainRules(its, rule, prefix.get(), false);
                        }
                        resultingRule = buildNontermRule(prefix->getGuard() & ar.newGuard);
                        if (Smt::check(resultingRule->getGuard(), its) != Smt::Sat) {
                            continue;
                        }
                    } else {
                        resultingRule = buildNontermRule(ar.newGuard);
                    }
                    res.rules.emplace_back(resultingRule.get());
                    res.proof.ruleTransformationProof(rule, "nonterm", resultingRule.get(), its);
                    res.proof.storeSubProof(ar.proof, "acceration calculus");
                } else {
                    option<Rule> resultingRule;
                    if (vb > 0) {
                        option<Rule> prefix = rule;
                        for (unsigned i = 0; i < vb - 1; ++i) {
                            prefix = Chaining::chainRules(its, rule, prefix.get(), false);
                        }
                        resultingRule = Rule(rule.getLhsLoc(), prefix->getGuard() & ar.newGuard, ap->getAcceleratedCost(), rule.getRhsLoc(), ap->getClosedForm().get());
                    } else {
                        resultingRule = Rule(rule.getLhsLoc(), ar.newGuard, ap->getAcceleratedCost(), rule.getRhsLoc(), ap->getClosedForm().get());
                    }
                    const BoolExpr toCheck = resultingRule->getGuard()->subs({ap->getIterationCounter(), max(2u, ap->getValidityBound())});
                    if (Smt::check(toCheck, its) != Smt::Sat) {
                        continue;
                    }
                    res.proof.ruleTransformationProof(rule, "acceleration", resultingRule.get(), its);
                    res.proof.storeSubProof(ar.proof, "acceration calculus");
                    std::vector<Rule> instantiated = replaceByUpperbounds(ap->getIterationCounter(), resultingRule.get());
                    if (instantiated.empty()) {
                        res.rules.emplace_back(resultingRule.get());
                    } else {
                        for (const Rule &r: instantiated) {
                            res.proof.ruleTransformationProof(resultingRule.get(), "instantiation", r, its);
                            res.rules.emplace_back(r);
                        }
                    }
                }
            }
        }
    }
    return res;
}


Acceleration::Result LoopAcceleration::accelerate(ITSProblem &its, const LinearRule &rule, LocationIdx sink, Complexity cpx) {
    LoopAcceleration ba(its, rule, sink, cpx);
    return ba.run();
}
