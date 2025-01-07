#pragma once

#include <clang/Frontend/FrontendActions.h>
#include "clang/Frontend/CompilerInstance.h"

#include "consumer.hpp"

struct ControlFlowAction : clang::ASTFrontendAction
{
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &Compiler,
                                                                llvm::StringRef InFile)
  {
    return std::make_unique<ControlFlowConsumer>(&Compiler.getASTContext());
  }
};

struct AbreuAction : clang::ASTFrontendAction
{
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &Compiler,
                                                                llvm::StringRef InFile)
  {
    std::cout << "Hello!" << std::endl;
    return std::make_unique<AbreuConsumer>(&Compiler.getASTContext());
  }
};
