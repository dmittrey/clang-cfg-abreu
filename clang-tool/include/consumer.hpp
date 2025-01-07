#pragma once

#include "clang/AST/ASTConsumer.h"

#include "visitor.hpp"

#include <iostream>
#include <fstream>

struct ControlFlowConsumer : clang::ASTConsumer 
{
private:
  Visitor Visitor;

public:
  explicit ControlFlowConsumer(ASTContext *Context) : Visitor(Context) {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());

    std::ofstream ofstream("graph.dot");
    Visitor.Draw(ofstream);
  }
};

struct AbreuConsumer : clang::ASTConsumer 
{
private:
  Visitor Visitor;

public:
  explicit AbreuConsumer(ASTContext *Context) : Visitor(Context) {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());

    Visitor.Stats();
  }
};
