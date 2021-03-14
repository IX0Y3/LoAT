#include "smtfactory.hpp"
#include "../config.hpp"
#include "z3/z3.hpp"
#include "yices/yices.hpp"
#include "combined_solver.hpp"

std::unique_ptr<Smt> SmtFactory::solver(Smt::Logic logic, const VariableManager &varMan, uint timeout) {
    std::unique_ptr<Smt> res;
    switch (logic) {
    case Smt::QF_LA:
        res = std::unique_ptr<Smt>(new Yices(varMan, logic));
        break;
    case Smt::QF_NA:
        res = std::unique_ptr<Smt>(new CombinedSolver(new Yices(varMan, logic), new Z3(varMan)));
        break;
//    case Smt::QF_NA:
    case Smt::QF_ENA:
        res = std::unique_ptr<Smt>(new Z3(varMan));
        break;
    }
    res->setTimeout(timeout);
    return res;
}

std::unique_ptr<Smt> SmtFactory::modelBuildingSolver(Smt::Logic logic, const VariableManager &varMan, uint timeout) {
    std::unique_ptr<Smt> res = solver(logic, varMan, timeout);
    res->enableModels();
    return res;
}
