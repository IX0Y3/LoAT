#include "config.hpp"
#include <iostream>

namespace Config {

    namespace Output {
        bool PrintDependencyGraph {false};
    }

    namespace Input {
        Format format = Horn;
    }

    // Asymptotic complexity computation using limit problems
    namespace Limit {
        // Discard a limit problem of size >= ProblemDiscardSize in a non-final check if z3 yields "unknown"
        const unsigned int ProblemDiscardSize = 10;
        const unsigned Timeout = 500u;
    }

    namespace Analysis {

        std::vector<Mode> modes { Complexity, Termination, RelativeTermination, Safety };

        Mode mode;
        Engine engine;
        SmtSolver smtSolver = Heuristic;
        bool log = false;
        bool logPreproc = false;
        bool logAccel = false;
        bool model = false;
        Direction dir;

        std::string modeName(const Mode mode) {
            switch (mode) {
            case Complexity: return "complexity";
            case Termination: return "termination";
            case RelativeTermination: return "relative_termination";
            case Safety: return "safety";
            }
        }

        bool relative_termination() {
            return mode == RelativeTermination;
        }

        bool termination() {
            return mode == Termination;
        }

        bool complexity() {
            return mode == Complexity;
        }

        bool safety() {
            return mode == Safety;
        }

        bool tryNonterm() {
            return termination() || relative_termination() || complexity();
        }

        bool doLogAccel() {
            return log || logAccel;
        }

        bool doLogPreproc() {
            return log || logPreproc;
        }

    }

    Config::TRPConfig trp;

    namespace ABMC {
        bool blocking_clauses {true};
    }

    namespace Accel {
        bool non_linear {true};
    }

}
