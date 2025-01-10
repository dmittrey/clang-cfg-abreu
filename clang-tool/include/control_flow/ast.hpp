#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <stack>
#include <numeric>

#include "graphiz.hpp"

namespace cfg {

namespace ast {

std::string prettyStmt(clang::Stmt* stmt, clang::ASTContext* Context) {
    if (!stmt)
        return "";
    std::string res;
    llvm::raw_string_ostream OS(res);
    stmt->printPretty(OS, nullptr, Context->getLangOpts());
    return res;
}

bool ForReached = false;
std::stack<std::vector<graphiz::FlowNode*>> BreakSubjectT;
std::stack<std::vector<graphiz::FlowNode*>> BreakSubjectF;

void PushBreak() {
    ForReached = false;
    BreakSubjectT.push({});
    BreakSubjectF.push({});
}

void PushBreakSubjectT(graphiz::FlowNode* Subject) {
    assert(BreakSubjectT.empty() != true);
    BreakSubjectT.top().push_back(Subject);
}

void PushBreakSubjectF(graphiz::FlowNode* Subject) {
    assert(BreakSubjectF.empty() != true);
    BreakSubjectF.top().push_back(Subject);
}

std::stack<graphiz::Statement*> ContinueAssignee;
std::stack<graphiz::FlowNode*> ContinueSubject;

void PushContinueAsignee(graphiz::Statement* Asignee) {
    ContinueAssignee.push(Asignee);
}

void PopContinueAsignee() {
    assert(ContinueAssignee.empty() != true);

    ContinueAssignee.pop();
}

graphiz::Statement* TopContinueAsignee() {
    assert(ContinueAssignee.empty() != true);

    graphiz::Statement* TopAssignee = ContinueAssignee.top();
    assert(TopAssignee != nullptr);

    return TopAssignee;
};

void PushContinueSubject(graphiz::FlowNode* Subject) {
    ContinueSubject.push(Subject);
}

void PopContinueSubject() {
    assert(ContinueSubject.empty() != true);

    ContinueSubject.pop();
}

graphiz::FlowNode* TopContinueSubject() {
    assert(ContinueSubject.empty() != true);

    graphiz::FlowNode* TopAssignee = ContinueSubject.top();
    assert(TopAssignee != nullptr);

    return TopAssignee;
};

enum class CompoundType {
    If,
    Else
};

struct Node {
public:
    virtual bool IsContinue() const { return false; }
    virtual bool IsBreak() const { return false; }
    virtual bool IsFor() const { return false; }
    virtual bool IsIf() const { return false; }

public:
    virtual graphiz::FlowNode* FlowStart() const = 0;
    virtual std::vector<graphiz::FlowNode*> FlowEnd() const = 0;

public:
    virtual void Assign(graphiz::Statement* ContNode) { assert(false); };

public:
    virtual ~Node() {};
};

Node* CreateNode(clang::Stmt* Stmt, clang::ASTContext* Context, CompoundType Type = CompoundType::If);

struct Operator : Node {
private:
    graphiz::Statement* FlowNode = nullptr;

public:
    Operator(clang::Expr* Op, clang::ASTContext* Context) {
        FlowNode = new graphiz::Statement(prettyStmt(Op, Context));
        // std::cout << "CREATED OP" << prettyStmt(Op, Context) << std::endl;
    }

public:
    graphiz::FlowNode* FlowStart() const override {
        // std::cout << __LINE__ << std::endl;
        return FlowNode; 
    }

    std::vector<graphiz::FlowNode*> FlowEnd() const override {
        // std::cout << __LINE__ << std::endl;
        return {FlowNode};
    }

    void Assign(graphiz::Statement* ContNode) override {
        FlowNode->assign(ContNode);
    }
};

struct Return : Node {
private:
    graphiz::Statement* FlowNode = nullptr;

public:
    Return(clang::ReturnStmt* RetStmt, clang::ASTContext* Context) {
        FlowNode = new graphiz::Statement(prettyStmt(RetStmt, Context));
        // std::cout << "CREATED RET" << prettyStmt(RetStmt, Context) << std::endl;
    }

public:
    graphiz::FlowNode* FlowStart() const override { 
        // std::cout << __LINE__ << std::endl;
        return FlowNode;
    }

    std::vector<graphiz::FlowNode*> FlowEnd() const override {
        // std::cout << __LINE__ << std::endl;
        return {};
    }

    void Assign(graphiz::Statement* ContNode) override {
        FlowNode->assign(ContNode);
    }
};

struct Decl : Node {
private:
    graphiz::Statement* FlowNode = nullptr;

public:
    Decl(clang::DeclStmt* DeclStmt, clang::ASTContext* Context) {
        FlowNode = new graphiz::Statement("");

        for (auto Iter = DeclStmt->decl_begin(); Iter != DeclStmt->decl_end(); ++Iter) {
            if (clang::VarDecl* varDecl = llvm::dyn_cast<clang::VarDecl>(*Iter)) {
                std::string VarName = varDecl->getNameAsString();

                clang::Expr *InitExpr = varDecl->getInit();
                if (!InitExpr)
                    continue;
                std::string InitStr = prettyStmt(InitExpr, Context);

                FlowNode->sourceCode += VarName + " = " + InitStr + '\n';
            }
        }

        // std::cout << "CREATED DECL" << prettyStmt(DeclStmt, Context) << std::endl;
    }

public:
    graphiz::FlowNode* FlowStart() const override { 
        // std::cout << __LINE__ << std::endl;
        return FlowNode;
    }

    std::vector<graphiz::FlowNode*> FlowEnd() const override {
        // std::cout << __LINE__ << std::endl;
        return {FlowNode};
    }

    void Assign(graphiz::Statement* ContNode) override {
        FlowNode->assign(ContNode);
    }
};

struct Function : Node {
private:
    graphiz::Call* CallFlow = nullptr;

private:
    Node* Body = nullptr;

public:
    Function(clang::FunctionDecl* FuncDecl, clang::ASTContext* Context) {
        std::vector<std::string> CallParams = {};
        for (auto iter = FuncDecl->param_begin(); iter != FuncDecl->param_end(); ++iter) {
            CallParams.push_back((*iter)->getName().data());
        }
        std::string CallName = FuncDecl->getNameInfo().getAsString();

        CallFlow = new graphiz::Call(CallName, CallParams);
        Body = CreateNode(FuncDecl->getBody(), Context);

        if (!Body)
            throw std::exception();

        if (Body->FlowStart())
            CallFlow->assign(Body->FlowStart());
    }

    graphiz::FlowNode* FlowStart() const override { 
        // std::cout << __LINE__ << std::endl;
        return CallFlow;
    }

    std::vector<graphiz::FlowNode*> FlowEnd() const override {
        // std::cout << __LINE__ << std::endl;
        return Body->FlowEnd();
    }
};

struct Continue : Node {
public:
    bool IsContinue() const override { return true; }

    graphiz::FlowNode* FlowStart() const override { 
        // std::cout << __LINE__ << std::endl;
        return nullptr;
    }

    std::vector<graphiz::FlowNode*> FlowEnd() const override {
        // std::cout << __LINE__ << std::endl;
        return {};
    }
};

struct Break : Node {
public:
    bool IsBreak() const override { return true; }

    graphiz::FlowNode* FlowStart() const override { 
        // std::cout << __LINE__ << std::endl;
        return nullptr;
    }

    std::vector<graphiz::FlowNode*> FlowEnd() const override {
        // std::cout << __LINE__ << std::endl;
        return {};
    }
};

struct If : Node {
private:
    graphiz::Condition* CondFlow;

private:
    Node* Then = nullptr;
    Node* Else = nullptr;

private:
    void SetThen(Node* Then_) {
        assert(Then_ != nullptr);
        Then = Then_;

        if (Then->IsContinue()) {
            CondFlow->assignT(TopContinueAsignee());
            return;
        }

        if (Then->IsBreak()) {
            PushBreakSubjectT(CondFlow);
            return;
        }

        // Empty Compund case
        if (Then->FlowStart())
            CondFlow->assignT(Then->FlowStart());
    }

    void SetElse(Node* Else_) {
        assert(Else_ != nullptr);
        Else = Else_;

        if (Else->IsContinue()) {
            CondFlow->assignF(TopContinueAsignee());
            return;
        }

        if (Else->IsBreak()) {
            PushBreakSubjectF(CondFlow);
            return;
        }

        if (Else->FlowStart())
            CondFlow->assignF(Else->FlowStart());
    }

public:
    If(clang::IfStmt *IfStmt, clang::ASTContext* Context) {
        clang::Expr *IfCond = IfStmt->getCond();
        if (!IfCond)
            throw std::exception();

        CondFlow = new graphiz::Condition(prettyStmt(IfCond, Context));

        PushContinueSubject(CondFlow);
        
        if (IfStmt->getThen())
            SetThen(CreateNode(IfStmt->getThen(), Context, CompoundType::If));
        if (IfStmt->getElse())
            SetElse(CreateNode(IfStmt->getElse(), Context, CompoundType::Else));
        
        PopContinueSubject();

        // std::cout << "CREATED IF" << prettyStmt(IfStmt, Context) << std::endl;
    }

    bool IsIf() const override { return true; }

    graphiz::FlowNode* FlowStart() const override { 
        std::cout << __LINE__ << std::endl;
        return CondFlow;
    }

    std::vector<graphiz::FlowNode*> FlowEnd() const override {
        std::cout << __LINE__ << std::endl;
        std::vector<graphiz::FlowNode*> res;

        auto ThenEnds = Then->FlowEnd();
        res.insert(res.end(), ThenEnds.begin(), ThenEnds.end());
        
        if (Else) {
            auto ElseEnds = Else->FlowEnd();
            res.insert(res.end(), ElseEnds.begin(), ElseEnds.end());
        }
        else {
            res.push_back(CondFlow);
        }

        return res;
    }
};

struct For : Node {
public:
    graphiz::Statement* InitFlow;
    graphiz::Condition* CondFlow;
    graphiz::Statement* IncFlow;

private:
    Node* Body = nullptr;

private:
    void SetBody(Node* Body_) {
        Body = Body_;

        if (Body->FlowStart()) {
            CondFlow->assignT(Body->FlowStart());

            auto Ends = Body->FlowEnd();
            for (auto* End : Ends) {
                std::cout << "Assign END" << Ends.size() << std::endl;
                End->assign(IncFlow);
            }
        }
        // Empty braces
        else {
            CondFlow->assignT(IncFlow);
        }

        auto Ends = Body->FlowEnd();
        for (auto* End : Ends) {
            std::cout << "Assign END" << Ends.size() << std::endl;
            End->assign(IncFlow);
        }
    }

public:
    For(clang::ForStmt *ForStmt, clang::ASTContext* Context) {
        auto *InitStmt = ForStmt->getInit();
        if (!InitStmt)
            throw std::exception();
        auto *CondExpr = ForStmt->getCond();
        if (!CondExpr)
            throw std::exception();
        auto *IncExpr = ForStmt->getInc();
        if (!IncExpr)
            throw std::exception();
        auto *BodyStmt = ForStmt->getBody();
        if (!BodyStmt)
            throw std::exception();

        InitFlow = new graphiz::Statement(prettyStmt(InitStmt, Context));
        CondFlow = new graphiz::Condition(prettyStmt(CondExpr, Context));
        IncFlow = new graphiz::Statement(prettyStmt(IncExpr, Context));
        
        PushBreak();
        PushContinueAsignee(IncFlow);
        PushContinueSubject(CondFlow);
        SetBody(CreateNode(BodyStmt, Context, CompoundType::If));
        PopContinueSubject();
        PopContinueAsignee();

        InitFlow->assign(CondFlow);
        IncFlow->assign(CondFlow);

        // std::cout << "CREATED FOR" << prettyStmt(ForStmt, Context) << std::endl;
    }

    bool IsFor() const override { return true; }

    graphiz::FlowNode* FlowStart() const override {
        // std::cout << __LINE__ << std::endl; 
        return InitFlow;
    }

    std::vector<graphiz::FlowNode*> FlowEnd() const override {
        // std::cout << __LINE__ << std::endl;
        return {CondFlow};
    }
};

struct Compound : Node {
private:
    Node* StartNode = nullptr;
    Node* LastNode = nullptr;

private:
    CompoundType Type;

private:
    bool PushStmt(Node* Stmt) {
        if (!Stmt) {
            // std::cout << "Push Unknown" << std::endl;
            return true;
        }
        if (Stmt->IsContinue()) {
            if (LastNode)
                LastNode->Assign(TopContinueAsignee());
            else {
                if (Type == CompoundType::If)
                    TopContinueSubject()->assignT(TopContinueAsignee());
                else
                    TopContinueSubject()->assignF(TopContinueAsignee());
            }
            return false;
        }
        if (Stmt->IsBreak()) {
            // assert(false);
            if (LastNode) {
                // assert(false);
                PushBreakSubjectT(LastNode->FlowStart());
                PushBreakSubjectF(LastNode->FlowStart());
                // assert(false);
            }
            else {
                if (Type == CompoundType::If)
                    PushBreakSubjectT(TopContinueSubject());
                else
                    PushBreakSubjectF(TopContinueSubject());
            }
            // assert(false);
            return false;
        }

        // assert(false);
        // Yo prevent fall in braces if in current scope(without breakSubjcts in collection)
        if (!BreakSubjectT.empty() && !BreakSubjectT.top().empty() && !Stmt->IsIf() && ForReached) {
            std::cout << "Assign Break (enter)" << std::endl;
            if (Stmt->IsFor()) {
                auto* ForStmt = static_cast<ast::For*>(Stmt);
                for (auto Subj : BreakSubjectT.top())
                    Subj->assignT(ForStmt->IncFlow);
            }
            else {
                for (auto Subj : BreakSubjectT.top())
                    Subj->assignT(Stmt->FlowStart());
            }
            BreakSubjectT.pop();
            std::cout << "Assign Break (exit)" << std::endl;
        }
        if (!BreakSubjectF.empty() && !BreakSubjectF.top().empty() && !Stmt->IsIf() && ForReached) {
            std::cout << "Assign Break (enter)" << std::endl;
            if (Stmt->IsFor()) {
                auto* ForStmt = static_cast<ast::For*>(Stmt);
                for (auto Subj : BreakSubjectF.top())
                    Subj->assignF(ForStmt->IncFlow);
            }
            else {
                for (auto Subj : BreakSubjectF.top())
                    Subj->assignF(Stmt->FlowStart());
            }
            BreakSubjectF.pop();
            std::cout << "Assign Break (exit)" << std::endl;
        }
        // assert(false);

        if (Stmt->IsFor())
            ForReached = true;

        if (!StartNode)
            StartNode = Stmt;

        if (LastNode)
            for (auto* End : LastNode->FlowEnd())
                End->assign(Stmt->FlowStart());

        LastNode = Stmt;

        return true;
    }

public:
    Compound(clang::CompoundStmt *CompoundStmt, clang::ASTContext* Context, CompoundType Type_) : Type(Type_) {
        for (auto Iter = CompoundStmt->body_begin(); Iter != CompoundStmt->body_end(); ++Iter) {
            // std::cout << "PUSH STMT" << prettyStmt(*Iter, Context) << std::endl;
            if (PushStmt(CreateNode(*Iter, Context)) == false) {
                LastNode = nullptr;
                break;
            }
        }
        // std::cout << "CREATED COMP" << prettyStmt(CompoundStmt, Context) << std::endl;
    }

    graphiz::FlowNode* FlowStart() const override {
        // Empty Compound case
        if (!StartNode)
            return nullptr;

        // std::cout << __LINE__ << StartNode->FlowStart()->getNodeLabel() << std::endl;
        return StartNode->FlowStart();
    }

    std::vector<graphiz::FlowNode*> FlowEnd() const override {
        // Empty Compound case
        if (!LastNode)
            return {};

        // std::cout << __LINE__ << LastNode->FlowStart()->getNodeLabel() << std::endl;
        return LastNode->FlowEnd();
    }
};

Node* CreateNode(clang::Stmt* Stmt, clang::ASTContext* Context, CompoundType Type) {
    assert(Stmt != nullptr);
    // std::cout << "CREATE" << prettyStmt(Stmt, Context) << std::endl;
    if (clang::BinaryOperator* BinOp = llvm::dyn_cast<clang::BinaryOperator>(Stmt)) {
        // std::cout << "CREATE BINOP" << prettyStmt(Stmt, Context) << std::endl;
        return new Operator(BinOp, Context);
    }
    if (clang::UnaryOperator* UnOp = llvm::dyn_cast<clang::UnaryOperator>(Stmt)) {
        // std::cout << "CREATE UNOP" << prettyStmt(Stmt, Context) << std::endl;
        return new Operator(UnOp, Context);
    }
    if (clang::ReturnStmt* RetStmt = llvm::dyn_cast<clang::ReturnStmt>(Stmt)) {
        // std::cout << "CREATE RET" << prettyStmt(Stmt, Context) << std::endl;
        return new Return(RetStmt, Context);
    }
    if (clang::DeclStmt* DeclStmt = llvm::dyn_cast<clang::DeclStmt>(Stmt)) {
        // std::cout << "CREATE DECL" << prettyStmt(Stmt, Context) << std::endl;
        return new Decl(DeclStmt, Context);
    }
    if (clang::BreakStmt* BreakStmt = llvm::dyn_cast<clang::BreakStmt>(Stmt)) {
        // std::cout << "CREATE BREAK" << prettyStmt(Stmt, Context) << std::endl;
        return new Break();
    }
    if (clang::ContinueStmt* ContinueStmt = llvm::dyn_cast<clang::ContinueStmt>(Stmt)) {
        // std::cout << "CREATE CONT" << prettyStmt(Stmt, Context) << std::endl;
        return new Continue();
    }
    if (clang::CompoundStmt* CompoundStmt = llvm::dyn_cast<clang::CompoundStmt>(Stmt)) {
        // std::cout << "CREATE CMPD" << prettyStmt(Stmt, Context) << std::endl;
        return new Compound(CompoundStmt, Context, Type);
    }
    if (clang::IfStmt* IfStmt = llvm::dyn_cast<clang::IfStmt>(Stmt)) {
        // std::cout << "CREATE IF" << prettyStmt(Stmt, Context) << std::endl;
        return new If(IfStmt, Context);
    }
    if (clang::ForStmt* ForStmt = llvm::dyn_cast<clang::ForStmt>(Stmt)) {
        // std::cout << "CREATE FOR" << prettyStmt(Stmt, Context) << std::endl;
        return new For(ForStmt, Context);
    }

    // throw std::exception();
    return nullptr;
}

}
}
