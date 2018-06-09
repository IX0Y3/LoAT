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

#ifndef PREPROCESS_H
#define PREPROCESS_H

#include "its/rule.h"
#include "its/variablemanager.h"


/**
 * Methods useful for preprocessing / simplifying the transitions
 */
namespace Preprocess
{
    /**
     * Main preprocessing function which combines the methods below in a suitable way.
     * Calls simplifyGuardBySmt, so this method involves many smt queries!
     *
     * @param rule The rule to be simplified, is modified.
     * @returns true iff rule was modified
     */
    bool preprocessRule(const VarMan &varMan, Rule &rule);

    /**
     * A simpler/cheaper version of preprocessRule without any smt queries.
     */
    bool simplifyRule(const VarMan &varMan, Rule &rule);

    /**
     * Simplifies the guard by dropping trivial constraints and constraints
     * which are (syntactically!) implied by one of the other constraints.
     * E.g. "x+1 >= x" is trivially true and "x > 1" implies "x > 0",
     * whereas "x^2 >= 0" is not recognized by the syntactic check.
     *
     * This method does not involve any z3 queries, so it only checks for
     * syntactically similar terms. The complexity is quadratic, but does
     * not involve any z3 queries.
     *
     * @return true iff the given guard was modified (some constraints were removed)
     */
    bool simplifyGuard(GuardList &guard);

    /**
     * Performs z3 queries to remove constraints which are implied by the previous
     * constraints. This involves a linear number of queries and is thus rather slow.
     *
     * @return true iff the given guard was modified (some constraints were removed)
     */
    bool simplifyGuardBySmt(GuardList &guard);

    /**
     * NOTE: Use simplifyGuardBySmt instead, unless this removes too many constraints!
     * Removes terms for which stronger variants appear in the guard, i.e. x >= 0, x > 0 --> x > 0
     * @note this _does_ involve many SMT queries (though only for every pair, transitivity is not checked)
     * @return true iff guard was modified
     */
    bool removeWeakerGuards(GuardList &guard);

    /**
     * Removes trivial updates of the form x <- x.
     * @return true iff update was modified
     */
    bool removeTrivialUpdates(const VarMan &varMan, UpdateMap &update);

    /**
     * Tries to remove as many temporary variables from update right-hand sides
     * and the guard as possible. Temporary variables are eliminated by equality propagation
     * (e.g. for free == 2*x) and transitive elimination (e.g. a <= free <= b becomes a <= b).
     * @param rule the rule, modified
     * @return true iff rule was modified
     */
    bool eliminateTempVars(const VarMan &varMan, Rule &rule);
}

#endif // PREPROCESS_H
