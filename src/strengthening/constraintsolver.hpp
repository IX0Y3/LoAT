/*  This file is part of LoAT.
 *  Copyright (c) 2019 Florian Frohn
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

#ifndef LOAT_STRENGTHENING_CONSTRAINT_SOLVER_H
#define LOAT_STRENGTHENING_CONSTRAINT_SOLVER_H

#include "types.hpp"
#include "templates.hpp"

namespace strengthening {

    class ConstraintSolver {

    public:

        static const option<Invariants> solve(
                const RuleContext &ruleCtx,
                const MaxSmtConstraints &constraints,
                const Templates &templates,
                Z3Context &z3Ctx);

    private:

        const RuleContext &ruleCtx;
        const MaxSmtConstraints &constraints;
        const Templates &templates;
        Z3Context &z3Ctx;

        ConstraintSolver(
                const RuleContext &ruleCtx,
                const MaxSmtConstraints &constraints,
                const Templates &templates,
                Z3Context &z3Ctx);

        const option<Invariants> solve() const;

        const GuardList instantiateTemplates(const z3::model &model) const;

        const option<Invariants> splitInitiallyValid(const GuardList &invariants) const;

    };

}


#endif //LOAT_STRENGTHENING_CONSTRAINT_SOLVER_H
