#include "config.hpp"
#include <iostream>

/**
 * Global settings and constants.
 *
 * Variables which are "const" should mostly not be changed.
 * The other variables may be changed (e.g., to choose which heuristics are used)
 * and may be controlled via command line options.
 *
 * See the source file for documentation (since this is also where the default values are defined)
 */
namespace Config {

    namespace Output {
        // Whether to enable colors in the proof output
        bool Colors {true};
        bool PrintDependencyGraph {false};
    }

    namespace Input {
        Format format = Koat;
    }

    // Asymptotic complexity computation using limit problems
    namespace Limit {
        // Discard a limit problem of size >= ProblemDiscardSize in a non-final check if z3 yields "unknown"
        const unsigned int ProblemDiscardSize = 10;
        const unsigned Timeout = 500u;
    }

    namespace Analysis {

        std::vector<Mode> modes { Complexity, Termination, Recurrence, Safety };

        Mode mode = Complexity;
        Engine engine;
        SmtSolver smtSolver = Heuristic;
        bool log = false;
        bool logPreproc = false;
        bool logAccel = false;
        bool model = false;
        Direction dir = Forward;
        std::string filename;

        std::string modeName(const Mode mode) {
            switch (mode) {
            case Complexity: return "complexity";
            case Termination: return "termination";
            case Recurrence: return "recurrence";
            case Safety: return "safety";
            }
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
            return termination() || complexity();
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

}
