#ifndef REDLOGPARSEVISITOR_H
#define REDLOGPARSEVISITOR_H

#include "redlogVisitor.h"
#include "../../util/exceptions.hpp"
#include "../../expr/boolexpr.hpp"
#include "../../its/variablemanager.hpp"

class RedlogParseVisitor: redlogVisitor
{
    enum BinOp {
        Minus, Exp
    };

    enum CAOp {
        Times, Plus
    };

    VariableManager &varMan;

    RedlogParseVisitor(VariableManager &varMan);

    virtual antlrcpp::Any visitMain(redlogParser::MainContext *ctx) override;
    virtual antlrcpp::Any visitExpr(redlogParser::ExprContext *ctx) override;
    virtual antlrcpp::Any visitCaop(redlogParser::CaopContext *ctx) override;
    virtual antlrcpp::Any visitBinop(redlogParser::BinopContext *ctx) override;
    virtual antlrcpp::Any visitFormula(redlogParser::FormulaContext *ctx) override;
    virtual antlrcpp::Any visitLit(redlogParser::LitContext *ctx) override;
    virtual antlrcpp::Any visitBoolop(redlogParser::BoolopContext *ctx) override;
    virtual antlrcpp::Any visitRelop(redlogParser::RelopContext *ctx) override;

public:

    EXCEPTION(ParseError, CustomException);

    static BoolExpr parse(std::string str, VariableManager &varMan);

};

#endif // REDLOGPARSEVISITOR_H
