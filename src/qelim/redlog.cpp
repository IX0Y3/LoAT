#include <future>

#include "redlog.hpp"
#include "../parser/redlog/redlogparsevisitor.h"
#include "../config.hpp"

RedProc initRedproc() {
    std::string path = std::getenv("PATH");
    std::replace(path.begin(), path.end(), ':', ' ');
    std::vector<std::string> array;
    std::stringstream ss(path);
    std::string temp;
    while (ss >> temp) array.push_back(temp);
    for (const std::string &p: array) {
        std::string redcsl = p + "/redcsl";
        std::ifstream file(redcsl);
        if (file.is_open()) {
            return RedProc_new(redcsl.c_str());
        }
    }
    throw Redlog::RedlogError("couldn't find redcsl binary");
}

RedProc Redlog::process() {
    static RedProc process = initRedproc();
    return process;
}

void Redlog::init() {
    if (Config::Qelim::useRedlog) {
        const char* cmd = "rlset r;";
        RedAns output = RedAns_new(process(), cmd);
        if (output->error) {
            RedProc_error(process(), cmd, output);
        }
        RedAns_delete(output);
    }
}

void Redlog::exit() {
    if (Config::Qelim::useRedlog) {
        RedProc_delete(process());
    }
}

option<Qelim::Result> Redlog::qe(const QuantifiedFormula &qf) {
    Proof proof;
    const auto p = qf.simplify().normalizeVariables(varMan);
    const QuantifiedFormula normalized = p.first;
    const Subs denormalization = p.second;
    if (normalized.isTiviallyTrue()) {
        proof.append("trivial");
        return Result(True, proof, true);
    } else if (normalized.isTiviallyFalse()) {
        proof.append("trivial");
        return Result(False, proof, true);
    }
    std::string command = "rlqe(" + normalized.toRedlog() + ");";
    auto qe = std::async([command]{return RedAns_new(process(), command.c_str());});
    if (qe.wait_for(std::chrono::milliseconds(1000)) != std::future_status::timeout) {
        RedAns output = qe.get();
        if (output->error) {
            RedProc_error(process(), command.c_str(), output);
            RedAns_delete(output);
        } else {
            std::string str = output->result;
            try {
                BoolExpr res = RedlogParseVisitor::parse(str, varMan);
                RedAns_delete(output);
                proof.append("QE via Redlog");
                return Result(res->simplify()->subs(denormalization), proof, false);
            } catch (const RedlogParseVisitor::ParseError &e) {
                std::cerr << e.what() << std::endl;
            }
        }
    }
    return {};
}
