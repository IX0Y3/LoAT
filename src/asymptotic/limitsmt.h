#ifndef LIMITSMT_H
#define LIMITSMT_H

#include "expr/expression.h"
#include "its/variablemanager.h"
#include "limitproblem.h"

namespace LimitSmtEncoding {
    /**
     * Tries to solve the given limit problem by an encoding into a SMT query.
     * @returns the found solution (if any), the limit problem is not modified.
     */
    option<GiNaC::exmap> applyEncoding(const LimitProblem &currentLP, const Expression &cost,
                                       const VarMan &varMan, bool finalCheck, Complexity currentRes);
}

#endif //LIMITSMT_H
