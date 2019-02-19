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

#ifndef FARKAS_H
#define FARKAS_H

#include "expr/expression.h"
#include "z3/z3context.h"
#include <vector>


/**
 * Implementation of Farkas' lemma.
 *
 * The lemma is used to transform universal quantification to an equivalent formula
 * that only uses existential quantification. When we search for a metering function,
 * we have to existentially quantify over the function's coefficients (since we want to finde them).
 * Hence applying Farkas' lemma helps us to avoid mixing quantifiers (which makes the z3 query much faster).
 *
 * The relevant version of Farkas lemma:
 *   Forall x: (A*x <= b implies c*x <= delta) can be rewritten as:
 *   Exists l: l >= 0, l^T * A = c^T, l^T * b <= delta (we refer to l as lambda in the code)
 *
 * In our context, x are variables, A and b represent guard/update, c the metering function's coefficients.
 */
namespace FarkasLemma {

    /**
     * Applies farkas lemma to transform the given constraints into z3 constraints.
     *
     * We represent the inequations "A*x <= b" by a list of Expressions,
     * which must be of the form "linear <= constant".
     *
     * The lists vars and coeffs must be of the same size and represent the variables
     * that (might) appear in the metering function and their corresponding coefficient symbols.
     * The absolute coefficient c0 is passed separately (since it does not belong to any variable).
     *
     * @note: The constraints may contain more variables (which are not contained in var).
     * To comply with the requirements of Farkas lemma, the coefficients for these extra
     * variables are simply set to zero (we need coefficients for every variable, as we have to compute c*x).
     *
     * @param constraints List of constraints of the form "linear term <= constant" (representing "A * x <= b")
     * @param vars List of variables ("x" or a subset of "x", where "x" are all variables in `constraints`)
     * @param coeffs List of z3 symbols for the coefficients (must be the same size as vars)
     * @param c0 The z3 symbol for the absolute coefficient
     * @param delta Integer value that is used as "delta" in Farkas lemma
     * @param context the Z3 context to create variables
     *
     * @return the resulting z3 expression (without quantifiers, as all variables are existentially quantified)
     */
    z3::expr apply(const std::vector<Expression> &constraints,
                   const std::vector<ExprSymbol> &vars,
                   const std::vector<z3::expr> &coeffs,
                   z3::expr c0,
                   int delta,
                   Z3Context &context,
                   const ExprSymbolSet &params = ExprSymbolSet(),
                   const Z3Context::VariableType &lambdaType = Z3Context::Real);

    const std::vector<z3::expr> apply(
            const std::vector<Expression> &premise,
            const std::vector<Expression> &conclusion,
            const ExprSymbolSet &vars,
            const ExprSymbolSet &params,
            Z3Context &context,
            const Z3Context::VariableType &lambdaType = Z3Context::Real);

    const z3::expr apply(
            const std::vector<Expression> &premise,
            const Expression &conclusion,
            const ExprSymbolSet &vars,
            const ExprSymbolSet &params,
            Z3Context &context,
            const Z3Context::VariableType &lambdaType = Z3Context::Real);

};

#endif // FARKAS_H
