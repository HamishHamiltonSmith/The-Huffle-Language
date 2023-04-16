#pragma once

#include "token.hpp"
#include "hcall.hpp"
#include <any>
#include <vector>

class Binary;
class Grouping;
class Literal; 
class Unary;
class Expression;
class Print;
class Expr;
class Variable;
class Var;
class Assignment;
class Block;
class Conditional;
class CWhile;
class Call;

enum LiteralType {
    INT, STR
};

enum VariableType {
    VAR, FUN
};


struct ExprVisitor {
    virtual std::any visitBinaryExpr(Binary* expr) = 0;
    virtual std::any visitGroupingExpr(Grouping* expr) = 0;
    virtual std::any visitLiteralExpr(Literal* expr) = 0;
    virtual std::any visitUnaryExpr(Unary* expr) = 0;
    virtual std::any visitVariableExpr(Variable* expr)=0;
    virtual std::any visitAssignmentExpr(Assignment* expr)=0;
    virtual std::any visitCallableExpr(Call* expr)=0;
};

struct StmtVisitor {
    virtual std::any visitExpressionStmt(Expression* expr)=0;
    virtual std::any visitPrintStmt(Print* expr)=0;
    virtual std::any visitVarStmt(Var* expr)=0;
    virtual std::any visitBlockStmt(Block* expr)=0;
    virtual std::any visitConditionalStmt(Conditional* expr)=0;
    virtual std::any visitCWhileStmt(CWhile* expr)=0;
};


struct Stmt {
    public:
    virtual std::any accept(StmtVisitor* v)=0;
};

class Expression : public Stmt {
    public:
    Expr* expression;

    Expression (Expr* expression){
        this->expression = expression;
    }

    std::any accept(StmtVisitor* v) { 
        return v->visitExpressionStmt(this);
    }
};

class Print : public Stmt {
    public:
    Expr* expression;

    Print (Expr* expression){
        this->expression = expression;
    }

    std::any accept(StmtVisitor* v) {
        return v->visitPrintStmt(this);
    }
};

class Var : public Stmt {
    public:
    Expr* initialiser;
    Token name;

    Var (Token name,  Expr* initialiser) {
        this->name = name;
        this->initialiser = initialiser;
    }

    std::any accept(StmtVisitor* v) {
        return v->visitVarStmt(this);
    }
};

class Block : public  Stmt {
    public:
    std::vector<Stmt*> statements;

    Block(std::vector<Stmt*> statements) {
        this->statements = statements;
    }
    std::any accept(StmtVisitor* v) {
        return v->visitBlockStmt(this);
    }
};

class Conditional : public Stmt {
    public:
    Expr* condition;
    Stmt* thenBranch;
    std::vector<Conditional*> elfs;
    Stmt* elseBranch;

    Conditional(Expr* condition, Stmt* thenBranch, Stmt* elseBranch = nullptr) {
        this->condition = condition;
        this->thenBranch = thenBranch;
        this->elfs = std::vector<Conditional*>(0);
        this->elseBranch=elseBranch;
    }

    Conditional(Expr* condition, Stmt* thenBranch, std::vector<Conditional*> elfs, Stmt* elseBranch = nullptr) {
        this->condition = condition;
        this->thenBranch = thenBranch;
        this->elfs = elfs;
        this->elseBranch=elseBranch;
    }

    std::any accept(StmtVisitor* v) {
        return v->visitConditionalStmt(this);
    }
};

class CWhile : public Stmt{
    public:
    Expr* condition;
    Stmt* body; 

    CWhile(Expr* condition, Stmt* body) {
        this->condition = condition;
        this->body = body;
    }

    std::any accept(StmtVisitor* v) {
        return v->visitCWhileStmt(this);
    }
};


//Logical & Arithmetic Expressions
struct Expr {
    public:
    virtual std::any accept(ExprVisitor* v)=0;
};

class Binary : public Expr {
    public:
    Expr* left;
    Expr* right;
    Token op;

    Binary(Expr* l, Token op, Expr* r) {
        this->left = l;
        this->right = r;
        this->op = op;
    }

    std::any accept(ExprVisitor* v) {
        return v->visitBinaryExpr(this);
    }
};

class Assignment : public Expr {
    public:
    Expr* expression;
    Token name;

    Assignment(Token name, Expr* expression) {
        this->name = name;
        this->expression = expression;
    }  

    std::any accept(ExprVisitor* v){
        return v->visitAssignmentExpr(this);
    }
};

class Literal : public Expr {
    public:
    std::any value;
    LiteralType type;

    Literal(std::any a) {
        value = a;
    }

    std::any accept(ExprVisitor* v) {
        return v->visitLiteralExpr(this);
    }
};

class Variable : public Expr {
    public:
    Token name;

    Variable(Token name) {
        this->name = name;
    }

    std::any accept(ExprVisitor* v) {
        return v->visitVariableExpr(this);
    }
};

class Grouping : public Expr {
    public:
    Expr* value;

    Grouping(Expr* val) {
        this->value = val;
    }

    std::any accept(ExprVisitor* v) {
        return v->visitGroupingExpr(this);
    }
};

class Call : public Expr {
    public:
    std::vector<Expr*> args;
    Expr* callee;
    Token paren;

    Call (Expr* callee, std::vector<Expr*> args, Token paren) {
        this->callee = callee;
        this->args = args;
        this->paren = paren;
    }

    std::any accept(ExprVisitor* v) {
        return v->visitCallableExpr(this);
    }
};

class Unary : public Expr {
    public:
    Token op;
    Expr* right;
    

    Unary(Token op, Expr* right) {
        this->op = op;
        this->right = right;
    }

    std::any accept(ExprVisitor* v) {
        return v->visitUnaryExpr(this);
    }
};