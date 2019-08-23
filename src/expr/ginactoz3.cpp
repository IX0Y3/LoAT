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

#include "ginactoz3.h"

#include "global.h"
#include "z3/z3context.h"

using namespace std;
using namespace GiNaC;


GinacToZ3::GinacToZ3(Z3Context &context, bool useReals)
        : context(context), useReals(useReals)
{}

z3::expr GinacToZ3::convert(const GiNaC::ex &expr, Z3Context &context, bool useReals) {
    GinacToZ3 converter(context, useReals);
    z3::expr res = converter.convert_ex(expr);
    debugGinacToZ3("converted " << expr << " to " << res << " [context=" << &context << ", reals=" << useReals << "]");
    return res;
}

z3::expr GinacToZ3::convert_ex(const GiNaC::ex &e) {
    if (is_a<add>(e)) {
        return convert_add(e);

    } else if (is_a<mul>(e)) {
        return convert_mul(e);

    } else if (is_a<power>(e)) {
        return convert_power(e);

    } else if (is_a<numeric>(e)) {
        return convert_numeric(ex_to<numeric>(e));

    } else if (is_a<symbol>(e)) {
        return convert_symbol(ex_to<symbol>(e));

    } else if (is_a<relational>(e)) {
        return convert_relational(e);
    }

    stringstream ss;
    ss << "Error: GiNaC type not implemented for term: " << e << endl;
    throw GinacZ3ConversionError(ss.str());
}

z3::expr GinacToZ3::convert_add(const GiNaC::ex &e) {
    assert(e.nops() > 0);

    z3::expr res = convert_ex(e.op(0));
    for (unsigned int i=1; i < e.nops(); ++i) {
        res = res + convert_ex(e.op(i));
    }

    return res;
}

z3::expr GinacToZ3::convert_mul(const GiNaC::ex &e) {
    assert(e.nops() > 0);

    z3::expr res = convert_ex(e.op(0));
    for (unsigned int i=1; i < e.nops(); ++i) {
        res = res * convert_ex(e.op(i));
    }

    return res;
}

z3::expr GinacToZ3::convert_power(const GiNaC::ex &e) {
    assert(e.nops() == 2);

    //rewrite power as multiplication if possible, which z3 can handle much better (e.g x^3 becomes x*x*x)
    if (is_a<numeric>(e.op(1))) {
        numeric num = ex_to<numeric>(e.op(1));
        if (num.is_integer() && num.is_positive() && num.to_long() <= Config::Z3::MaxExponentWithoutPow) {
            int exp = num.to_int();
            z3::expr base = convert_ex(e.op(0));

            z3::expr res = base;
            while (--exp > 0) {
                res = res * base;
            }

            return res;
        }
    }

    //use z3 power as fallback (only poorly supported)
    return z3::pw(convert_ex(e.op(0)), convert_ex(e.op(1)));
}

z3::expr GinacToZ3::convert_numeric(const GiNaC::numeric &num) {
    assert(num.is_integer() || num.is_real());

    try {
        // convert integer either as integer or as reals (depending on settings)
        if (num.is_integer()) {
            if (useReals) {
                return context.real_val(num.to_long(),1);
            } else {
                return context.int_val(num.to_long());
            }
        }

        // always convert real numbers as reals
        return context.real_val(num.numer().to_long(),num.denom().to_long());

    } catch (...) {
        throw GinacZ3LargeConstantError("Numeric constant too large, cannot convert to z3");
    }
}

z3::expr GinacToZ3::convert_symbol(const GiNaC::symbol &e) {
    // if the symbol is already known, we re-use it (regardless of its type!)
    auto optVar = context.getVariable(e);
    if (optVar) {
        // Warn if the re-used variable does not have the correct type.
        // This feature might be useful if useReals = true, but we want some variables to be integers.
        // However, in most cases it is probably not intended, so we issue the warning.
        if (!Z3Context::isVariableOfType(optVar.get(), variableType())) {
            debugWarn("GinacToZ3: Re-using existing symbol of type: " << e << " (but it should have type: " << variableType() << ")");
        }

        return optVar.get();
    }

    // otherwise we add a fresh z3 variable and associate it with this symbol
    return context.addNewVariable(e, variableType());
}

z3::expr GinacToZ3::convert_relational(const GiNaC::ex &e) {
    assert(e.nops() == 2);

    z3::expr a = convert_ex(e.op(0));
    z3::expr b = convert_ex(e.op(1));

    if (e.info(info_flags::relation_equal)) return a == b;
    if (e.info(info_flags::relation_not_equal)) return a != b;
    if (e.info(info_flags::relation_less)) return a < b;
    if (e.info(info_flags::relation_less_or_equal)) return a <= b;
    if (e.info(info_flags::relation_greater)) return a > b;
    if (e.info(info_flags::relation_greater_or_equal)) return a >= b;

    unreachable();
}

Z3Context::VariableType GinacToZ3::variableType() const {
    return (useReals) ? Z3Context::Real : Z3Context::Integer;
}