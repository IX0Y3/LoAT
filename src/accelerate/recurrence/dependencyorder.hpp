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

#ifndef DEPENDENCYORDER_H
#define DEPENDENCYORDER_H

#include "../../its/rule.hpp"
#include "../../its/variablemanager.hpp"
#include "../../util/option.hpp"


/**
 * Functions to compute an ordering on updated variables,
 * such that we can solve recurrences in this order.
 *
 * For an update A := B+C, the recurrences for B and C must be known before
 * we can compute the recurrence for A. So B,C come before A in the ordering.
 *
 * Note that such an ordering does not always exist,
 * e.g., A := B+1, B := A+2.
 */
namespace DependencyOrder {
    /**
     * Tries to find an order to calculate recurrence equations.
     * Fails if there is a nontrivial set of variables whose updates depend on each other.
     * @return list indicating the order (if successful)
     */
    option<std::vector<VariableIdx>> findOrder(const VarMan &varMan, const UpdateMap &update);

    /**
     * Tries to find an order to calculate recurrence equations.
     *
     * If this is not possible, an optimization is used that assumes all conflicting variables are equal.
     * To this end, new constraints are added to the guard (of the form "A == B") and the update is simplified by
     * replacing variables accordingly (e.g., A/B).
     *
     * Note that the heuristic is not always applicable, so we can still fail to find an ordering.
     *
     * @return list indicating the order (if successful), both arguments may be modified
     */
    option<std::vector<VariableIdx>> findOrderWithHeuristic(const VarMan &varMan, UpdateMap &update, GuardList &guard);
};

#endif // DEPENDENCYORDER_H
