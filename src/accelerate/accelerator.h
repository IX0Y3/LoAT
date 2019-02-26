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

#ifndef ACCELERATE_H
#define ACCELERATE_H

#include "global.h"

#include "its/types.h"
#include "its/itsproblem.h"
#include "expr/expression.h"
#include "meter/metering.h"
#include "forward.h"


class Accelerator {
public:
    /**
     * Tries to accelerate all simple loops of the given location
     * by combining forward and backward acceleration techniques.
     * Also handles nesting and chaining of simple loops (if enabled).
     *
     * Successfully accelerated simple loops are removed afterwards.
     * rules for which acceleration failed may be kept (and are then also added to resultingRules).
     * This is intended for cases where the loop itself already has non-trivial complexity.
     *
     * @param resultingRules resulting (usually accelerated) rules are inserted (via their index).
     * @return true iff the ITS was modified
     *         (which is always the case if any simple loops were present)
     */
    static bool accelerateSimpleLoops(ITSProblem &its, LocationIdx loc, std::set<TransIdx> &resultingRules);

private:
    // Potential candidate for the inner loop when nesting two loops.
    // Inner loops are always accelerated loops, so this stores the original and the accelerated rule.
    // The information on the original rule is only used to avoid nesting a loop with itself
    struct InnerCandidate {
        TransIdx oldRule;
        TransIdx newRule;
    };

    // Potential candidate for the outer loop when nesting two loops.
    // Outer loops are always un-accelerated rules, so we just store an original rule here.
    struct OuterCandidate {
        TransIdx oldRule;
    };

private:
    Accelerator(ITSProblem &its, LocationIdx loc, std::set<TransIdx> &resultingRules);

    /**
     * Main function. Tries to accelerate and nest all loops
     * by calling the methods below in a suitable way.
     */
    void run();

    /**
     * Helper that calls Preprocess::simplifyRule
     * Returns true iff any rule was modified.
     */
    bool simplifySimpleLoops();

    /**
     * Tries to accelerate the given rule by forward acceleration and,
     * if this fails, by backward acceleration.
     * @returns The acceleration result (including accelerated rules, if successful)
     */
    ForwardAcceleration::Result tryAccelerate(const Rule &rule) const;


    /**
     * Tries to accelerate the given rule by calling tryAccelerate.
     * If this fails and the rule is nonlinear, we try to accelerate the rule again
     * after removing some of the right-hand sides. This may simplify the rule,
     * so acceleration might then be successful (maybe with a lower complexity).
     * Since we don't known which right-hand sides are causing trouble,
     * we try many combinations of removing right-hand sides, so this might be expensive.
     *
     * @note We also try to remove all but one right-hand side, so we reduce the nonlinear
     * rule to a linear rule which may lose the exponential complexity. But since we cannot
     * accelerate nonlinear rules by backward acceleration, this is still preferable to
     * just giving up.
     *
     * @returns If successful, the resulting accelerated rule(s). Otherwise,
     * the acceleration result from accelerating the original rule (before shortening).
     */
    ForwardAcceleration::Result accelerateOrShorten(const Rule &rule) const;


    /**
     * Adds the given rule to the ITS problem and to resultingRules.
     * @return The index of the added rule.
     */
    TransIdx addResultingRule(Rule rule);

    /**
     * Helper function that checks with a simple heuristic if the transitions might be nested loops
     * (this is done to avoid too many nesting attemts, as finding a metering function takes some time).
     */
    bool canNest(const LinearRule &inner, const LinearRule &outer) const;

    /**
     * Adds the given accelerated rule to the ITS.
     * Also tries to chain the rule `chain` in front of the accelerated rule (and adds the result, if any).
     * Takes care of proof output (the arguments inner, outer are only used for the output).
     */
    void addNestedRule(const LinearRule &accelerated, const LinearRule &chain,
                       TransIdx inner, TransIdx outer);

    /**
     * Tries to nest the given nesting candidates (i.e., rules).
     * Returns true if nesting was successful (at least one new rule was added).
     */
    bool nestRules(const Complexity &currentCpx, const InnerCandidate &inner, const OuterCandidate &outer);

    /**
     * Main implementation of nesting
     */
    void performNesting(std::vector<InnerCandidate> inner, std::vector<OuterCandidate> outer);

    /**
     * Removes all given loops, unless they are contained in keepRules.
     * Also prunes duplicate rules from resultingRules.
     */
    void removeOldLoops(const std::vector<TransIdx> &loops);

private:
    // The ITS problem. Accelerated rules are added to the ITS immediately,
    // but no rules are removed until the very end (end of run()).
    ITSProblem &its;

    // The location for which simple loops shall be accelerated.
    LocationIdx targetLoc;

    // Sink location for accelerated (nonlinear/nonterminating) rules
    LocationIdx sinkLoc;

    // The set of resulting rules.
    // These are all accelerated rules and some rules for which acceleration failed.
    std::set<TransIdx> &resultingRules;

    // All rules where acceleration failed, but where we want to keep the un-accelerated rule.
    std::set<TransIdx> keepRules;

    const ForwardAcceleration::Result strengthenAndAccelerate(const Rule &rule) const;
};

#endif // ACCELERATE_H
