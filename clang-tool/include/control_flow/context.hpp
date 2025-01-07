#pragma once

#include <map>

#include "ast.hpp"

namespace cfg {

struct Context {
public:
    ast::Node* Top = nullptr;

public:
    void Draw(std::ofstream &ofstream) {
        renderGraph(Top->FlowStart(), ofstream);
    }

public:
    void Push(ast::Function* Func) {
        if (!Top)
            Top = Func;
    }
};

}