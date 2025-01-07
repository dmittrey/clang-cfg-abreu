#pragma once

#include <unordered_set>
#include <sstream>
#include <fstream>

namespace cfg {

namespace graphiz {

struct FlowNode {
protected:
    FlowNode* endpointT_ = nullptr;
    FlowNode* endpointF_ = nullptr;

public:
    void assignT(FlowNode* endpointT) {
        // std::cout << "Assign true " << getNodeLabel() << " to " << endpointT->getNodeLabel() << std::endl;
        // assert(endpointT_ == nullptr);
        assert(endpointT != nullptr);
        endpointT_ = endpointT; 
        // std::cout << "Assign true " << getNodeLabel() << " to " << endpointT->getNodeLabel() << std::endl;
    }
    void assignF(FlowNode* endpointF) {
        assert(endpointF_ == nullptr);
        assert(endpointF != nullptr);
        endpointF_ = endpointF; 
        // std::cout << "Assign false " << getNodeLabel() << " to " << endpointF->getNodeLabel() << std::endl;
    }
    FlowNode* endpointT() const { return endpointT_; }
    FlowNode* endpointF() const { return endpointF_; }

public:
    virtual void assign(FlowNode* endpoint) = 0;

public:
    virtual std::string getNodeLabel() const = 0;
    virtual std::string getNodeShape() const = 0;

public:
    virtual ~FlowNode() {}
};

struct Call : FlowNode {
private:
    std::string name_;
    std::vector<std::string> params_;

public:
    explicit Call(const std::string& name, const std::vector<std::string>& params)
        : name_(name), params_(params) {}

public:
    void assign(FlowNode* endpoint) override {
        if (!endpointT_) {
            // std::cout << "Assign true " << getNodeLabel() << " to " << endpoint->getNodeLabel() << std::endl;
            endpointT_ = endpoint;
        }
        if (!endpointF_) {
            // std::cout << "Assign false " << getNodeLabel() << " to " << endpoint->getNodeLabel() << std::endl;
            endpointF_ = endpoint;
        }
    }

public:
    std::string getNodeLabel() const override {
        std::ostringstream oss;
        oss << name_ << "(";
        for (size_t i = 0; i < params_.size(); ++i) {
            oss << params_[i];
            if (i != params_.size() - 1) oss << ", ";
        }
        oss << ")";
        return oss.str();
    }

    std::string getNodeShape() const override { return "ellipse"; }
};

struct Statement : FlowNode {
public:
    std::string sourceCode;

public:
    explicit Statement(const std::string& sourceCode_) : sourceCode(sourceCode_) {}

public:
    void assign(FlowNode* endpoint) override {
        if (!endpointT_) {
            // std::cout << "Assign true " << getNodeLabel() << " to " << endpoint->getNodeLabel() << std::endl;
            endpointT_ = endpoint;
        }
        if (!endpointF_) {
            // std::cout << "Assign false " << getNodeLabel() << " to " << endpoint->getNodeLabel() << std::endl;
            endpointF_ = endpoint;
        }
    }

public:
    std::string getNodeLabel() const override { return sourceCode; }

    std::string getNodeShape() const override { return "rectangle"; }
};

struct Condition : FlowNode {
private:
    std::string sourceCode;

public:
    explicit Condition(const std::string& sourceCode)
        : sourceCode(sourceCode) {}

public:
    void assign(FlowNode* endpoint) override {
        if (!endpointT_) {
            // std::cout << "Assign true " << getNodeLabel() << " to " << endpoint->getNodeLabel() << std::endl;
            endpointT_ = endpoint;
        }
        if (!endpointF_) {
            // std::cout << "Assign false " << getNodeLabel() << " to " << endpoint->getNodeLabel() << std::endl;
            endpointF_ = endpoint;
        }
    }

public:
    std::string getNodeLabel() const override { return sourceCode; }

    std::string getNodeShape() const override { return "diamond"; }
};

void renderFlowNode(const FlowNode* Node, std::ostream& out, std::unordered_set<const FlowNode*>& visited) {
    if (!Node || visited.count(Node)) return;

    // Помечаем текущий узел как посещённый
    visited.insert(Node);

    // Получаем информацию об узле
    out << "    \"" << Node << "\" [shape=" << Node->getNodeShape()
        << ", label=\"" << Node->getNodeLabel() << "\"];\n";

    // Обрабатываем тип узла и его связи
    const auto* trueBranch = Node->endpointT();
    const auto* falseBranch = Node->endpointF();
    if (trueBranch && trueBranch == falseBranch) {
        out << "    \"" << Node << "\" -> \"" << trueBranch << "\";\n";
        renderFlowNode(trueBranch, out, visited);
        return;
    }
    if (trueBranch) {
        out << "    \"" << Node << "\" -> \"" << trueBranch << "\" [label=\"true\"];\n";
        renderFlowNode(trueBranch, out, visited);
    }
    if (falseBranch) {
        out << "    \"" << Node << "\" -> \"" << falseBranch << "\" [label=\"false\"];\n";
        renderFlowNode(falseBranch, out, visited);
    }
}


void renderGraph(const FlowNode* root, std::ostream& out) {
    std::unordered_set<const FlowNode*> visited;
    out << "digraph FlowGraph {\n";
    renderFlowNode(root, out, visited);
    out << "}\n";
}

}

}
