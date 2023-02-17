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

#include "main.hpp"

#include "itsparser.hpp"
#include "parser.hpp"
#include "chcparser.hpp"
#include "cintparser.hpp"
#include "config.hpp"
#include "proof.hpp"
#include "version.hpp"
#include "reachability.hpp"
#include "satisfiability.hpp"

#include <iostream>
#include <boost/algorithm/string.hpp>

using namespace std;

// Variables for command line flags
string filename;

void printHelp(char *arg0) {
    cout << "Usage: " << arg0 << " [options] <file>" << endl;
    cout << "Options:" << endl;
    cout << "  --proof-level <n>                                Detail level for proof output" << endl;
    cout << "  --plain                                          Disable colored output" << endl;
    cout << "  --mode <complexity|non_termination|reachability> Analysis mode" << endl;
    cout << "  --format <koat|its|horn>                         Input format" << endl;
}


void parseFlags(int argc, char *argv[]) {
    int arg=0;

    auto getNext = [&]() {
        if (arg < argc-1) {
            return argv[++arg];
        } else {
            cout << "Error: Argument missing for " << argv[arg] << endl;
            exit(1);
        }
    };

    while (++arg < argc) {
        if (strcmp("--help",argv[arg]) == 0) {
            printHelp(argv[0]);
            exit(1);
        } else if (strcmp("--proof-level",argv[arg]) == 0) {
            int proofLevel = atoi(getNext());
            if (proofLevel < 0) {
                cerr << "proof level must be non-negative, ignoring value " << proofLevel << endl;
            } else {
                Proof::setProofLevel(proofLevel);
            }
        } else if (strcmp("--plain",argv[arg]) == 0) {
            Config::Output::Colors = false;
            Proof::disableColors();
        } else if (strcmp("--reach-log", argv[arg]) == 0) {
            reachability::Reachability::log = true;
        } else if (strcmp("--sat-log", argv[arg]) == 0) {
            satisfiability::Satisfiability::log = true;
        } else if (strcmp("--mode", argv[arg]) == 0) {
            bool found = false;
            std::string str = getNext();
            for (const Config::Analysis::Mode mode: Config::Analysis::modes) {
                if (boost::iequals(str, Config::Analysis::modeName(mode))) {
                    Config::Analysis::mode = mode;
                    found = true;
                    break;
                }
            }
            if (!found) {
                cerr << "Unknown mode " << str << ", defaulting to " << Config::Analysis::modeName(Config::Analysis::mode) << endl;
            }
        } else if (strcmp("--format", argv[arg]) == 0) {
            std::string str = getNext();
            if (boost::iequals("koat", str)) {
                Config::Input::format = Config::Input::Koat;
            } else if (boost::iequals("its", str)) {
                Config::Input::format = Config::Input::Its;
            } else if (boost::iequals("horn", str)) {
                Config::Input::format = Config::Input::Horn;
            } else if (boost::iequals("c", str)) {
                Config::Input::format = Config::Input::C;
            } else {
                cout << "Error: unknown format " << str << std::endl;
                exit(1);
            }
        } else {
            if (!filename.empty()) {
                cout << "Error: additional argument " << argv[arg] << " (already got filename: " << filename << ")" << endl;
                exit(1);
            }
            filename = argv[arg];
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printHelp(argv[0]);
        return 1;
    }

    // Parse and interpret command line flags
    parseFlags(argc, argv);

    // Start parsing
    if (filename.empty()) {
        cerr << "Error: missing filename" << endl;
        return 1;
    }

    ITSProblem its;
    switch (Config::Input::format) {
    case Config::Input::Koat:
        its = parser::ITSParser::loadFromFile(filename);
        break;
    case Config::Input::Its:
        its = sexpressionparser::Parser::loadFromFile(filename);
        break;
    case Config::Input::Horn:
        its = hornParser::HornParser::loadFromFile(filename);
        break;
    case Config::Input::C:
        its = cintParser::CIntParser::loadFromFile(filename);
        break;
    default:
        std::cout << "Error: unknown format" << std::endl;
        exit(1);
    }

    // Start the analysis of the parsed ITS problem.
    // Skip ITS problems with nonlinear (i.e., recursive) rules.
    switch (Config::Analysis::mode) {
    case Config::Analysis::Complexity:
    case Config::Analysis::NonTermination:
    case Config::Analysis::Reachability:
        reachability::Reachability::analyze(its);
        break;
    case Config::Analysis::Satisfiability:
        satisfiability::Satisfiability::analyze(its);
        break;
    case Config::Analysis::CheckLinear:
        for (const auto &idx: its.getAllTransitions()) {
            const auto rule = its.getRule(idx);
            if (!rule.getGuard()->isLinear() || !rule.getUpdate().isLinear()) {
                std::cout << "NO" << std::endl;
                return 0;
            }
        }
        std::cout << "YES" << std::endl;
        return 0;
    default:
        throw std::invalid_argument("unsupported mode");
    }

    cout << "Build SHA: " << Version::GIT_SHA << (Version::GIT_DIRTY == "1" ? " (dirty)" : "") << endl;

    return 0;
}
