#pragma once

#include "theory.hpp"

namespace util {

    class RelevantVariables {

    public:

        static VarSet find(
                const VarSet &varsOfInterest,
                const Subs &up) {
            VarSet res = varsOfInterest;
            // Compute the closure of res under all updates and the guard
            VarSet todo = res;
            while (!todo.empty()) {
                VarSet next;
                for (const auto &x : todo) {
                    std::visit([&up, &next](const auto &x) {
                        const auto val {up.get(x)};
                        if (val != theory::toExpr(x)) {
                            theory::collectVars(val, next);
                        }
                    }, x);
                }
                todo.clear();
                for (const auto &var : next) {
                    if (!res.contains(var)) {
                        todo.insert(var);
                    }
                }
                // collect all variables from every iteration
                res.insertAll(todo);
            }
            VarSet symbols;
            for (const auto &x: res) {
                symbols.insert(x);
            }
            return symbols;
        }

    };

}
