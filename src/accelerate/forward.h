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

#ifndef FORWARD_H
#define FORWARD_H

#include "global.h"

#include "its/types.h"
#include "its/itsproblem.h"
#include "expr/expression.h"
#include "meter/metering.h"

#include "util/option.h"

/**
 * The classic acceleration technique by using metering functions.
 * This is applicable to for both linear and nonlinear rules (with several right-hand sides).
 */
namespace ForwardAcceleration {
    enum ResultKind {
        NoMetering, // No metering function could be found
        NonMonotonic, //
        NoClosedFrom,
        NonCommutative,
        TooComplicated, // Guard/update is too complicated (nonlinear or no dependency order)
        Success, // Rule was successfully accelerated or is non-terminating (then the cost is set to InftySymbol)
        SuccessWithRestriction // Success after a heuristic extended the guard (so it is now more restrictive)
    };

    struct MeteredRule {
        std::string info; // printable version of the metering function
        Rule rule;
        MeteredRule(std::string s, Rule r) : info(s), rule(r) {}
        MeteredRule appendInfo(const std::string &s) { return MeteredRule(info + s, rule); }
    };

    struct Result {
        ResultKind result;
        std::vector<MeteredRule> rules;
    };

    /**
     * Tries to accelerate the given rule, which must be a simple loop.
     * If no metering function is found in the first attempt, several heuristics are used to simplify the rule.
     * All resulting accelerated rules are returned (some heuristics may yield several rules).
     *
     * All resulting rules are linear.
     * If the original loop was linear, the result is still a simple loop, unless it is non-terminating.
     * If the original loop was nonlinear or found to be non-terminating, the resulting rules go to the given sink.
     */
    Result accelerate(VarMan &varMan, const Rule &rule, LocationIdx sink);

    /**
     * Like accelerateNonlinear, but does not invoke any heuristics (and is thus faster but less powerful).
     * The result is always a single accelerated rule (if acceleration was successful).
     */
    option<MeteredRule> accelerateFast(VarMan &varMan, const Rule &rule, LocationIdx sink);
};

#endif // FORWARD_H
