#pragma once

#include <vector>
#include <iostream>

#include "control_flow/context.hpp"
#include "abreu/context.hpp"

#include "clang/AST/RecursiveASTVisitor.h"

using namespace clang;

class Visitor : public RecursiveASTVisitor<Visitor>
{
private:
    ASTContext *Context;

private:
    cfg::Context CfgCtx;
    abreu::Context AbreuCtx;

public:
    Visitor(ASTContext *Context) : Context(Context) {}

    void Draw(std::ofstream &ofstream) {
        CfgCtx.Draw(ofstream);
    }

    void Stats() {
        AbreuCtx.Stats();
    }

public:
    bool VisitCXXRecordDecl(CXXRecordDecl* Record) {
        AbreuCtx.Push(new abreu::ast::Class(Record, Context));
        return true;
    }
    // bool VisitFunctionDecl(FunctionDecl *FuncDecl) {
        // CfgCtx.Push(new cfg::ast::Function(FuncDecl, Context));
        // return true;
    // }

    bool VisitTranslationUnitDecl(TranslationUnitDecl* stmt) {
        stmt->dump();

        return true;
    }
};