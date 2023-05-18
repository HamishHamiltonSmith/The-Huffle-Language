#pragma once
#include <cstdarg>
#include <sstream>
#include <string>
#include "enviroment.hpp"
#include "error.hpp"
#include "types.hpp"
#include "expr.hpp"
#include "hcall.hpp"

void addGlobal(Enviroment& env, std::string name, HCallable* callable) {
    env.define(name, callable);
}

void addGlobal(Enviroment& env, Token name, HCallable* callable) {
    env.define(name, callable);
}




Interpreter::Interpreter() {
    env = new Enviroment(false);
    global = env;

    addGlobal(*global, "in", new in());
    addGlobal(*global, "type", new type());
    addGlobal(*global, "toNum", new toNum());
    addGlobal(*global, "toStr", new toStr());
    addGlobal(*global, "len", new length());
    addGlobal(*global, "contains", new Contains());
    addGlobal(*global, "leave", new leave());
    addGlobal(*global, "sin", new Sin());
    addGlobal(*global, "cos", new Cos());
    addGlobal(*global, "tan", new Tan());
    addGlobal(*global, "atan", new Atan());
    addGlobal(*global, "acos", new Acos());
    addGlobal(*global, "asin", new Asin());
    addGlobal(*global, "sqrt", new Sqrt());
    addGlobal(*global, "pow", new Pow());
    addGlobal(*global, "floor", new Floor());
    addGlobal(*global, "ceil", new Ceil());
    addGlobal(*global, "round", new Round());
    addGlobal(*global, "log", new Log());
    addGlobal(*global, "wait", new Sleep());
    addGlobal(*global, "page", new CreatePage());
    addGlobal(*global, "updatePage", new UpdatePage());
    addGlobal(*global, "TEXT", new AddText());
    addGlobal(*global, "RECT", new AddRect());

    global->define("PI", (double)3.141592653589793);
    global->define("E", (double)2.7182818284);
}

//Statement Interpretation
std::any Interpreter::visitPrintStmt(Print* stmt) {
    std::cout << huff::anyToString(stmt->expression->accept(this)) << "\n";
    return NULL;
}

std::any Interpreter::visitVarStmt(Var* stmt) {
    //Store variable in eviroment map
    env->define(stmt->name, stmt->initialiser->accept(this));
    return NULL;
}

std::any Interpreter::visitBlockStmt(Block* stmt) {
    Enviroment* blockEnv = new Enviroment(this->env->isFunc, env);
    executeBlock(stmt, blockEnv);
    return NULL;
}

std::any Interpreter::visitConditionalStmt(Conditional* stmt) {
    if (isTruthy(stmt->condition->accept(this))) {
        stmt->thenBranch->accept(this);
    } else {
        for (Conditional* elfBranch : stmt->elfs) {
            if (isTruthy(elfBranch->condition->accept(this))) {
                elfBranch->thenBranch->accept(this);
            } else if (elfBranch->elseBranch != nullptr) {
                elfBranch->elseBranch->accept(this);
            }
        }

        if (stmt->elseBranch != nullptr) {
            stmt->elseBranch->accept(this);
        }
    } 

    return NULL;
}

std::any Interpreter::visitCWhileStmt(CWhile* stmt) {
    while (isTruthy(stmt->condition->accept(this))){
        stmt->body->accept(this);
    }

    return NULL;
}

std::any Interpreter::visitFunctionStmt(Func* stmt) {
    addGlobal(*global, stmt->name, new UDCallable(stmt, env));
    return NULL;
}

std::any Interpreter::visitClassStmt(Class* stmt) {
    return NULL;
}

std::any Interpreter::visitExpressionStmt(Expression* stmt) {
    stmt->expression->accept(this);
    return NULL;
}

std::any Interpreter::visitReturnStmt(Return* stmt) {
    if (env->isFunc) {
        return stmt->returnVal->accept(this);
    } else {
        throw new RuntimeError("Invalid use of return statement from outside function scope", stmt->line);
    }
}

//Expression Interpretation
std::any Interpreter:: visitLiteralExpr(Literal* expr) {
    return expr->value;
}

std::any Interpreter::visitGroupingExpr(Grouping* expr) {
    return expr->value->accept(this);
}

std::any Interpreter::visitUnaryExpr(Unary* expr) {
    std::any right = expr->right->accept(this);
    switch(expr->op.type) {
        case MINUS:
            castValid<double>(1,right);
            return -std::any_cast<double>(right);
        case EXL:
            return !isTruthy(right);  
    }

    return right;
}

std::any Interpreter::visitCallableExpr(Call* expr) {
    std::any callee = expr->callee->accept(this);

    std::vector<std::any> args;
    for (auto arg: expr->args) {
        args.push_back(arg->accept(this));
    }

    try {
        HCallable* func = std::any_cast<HCallable*>(callee);
        return func->call(this,expr->paren, args);
    } catch (std::bad_any_cast e) {
        throw(new RuntimeError("Illegal use of call operater on non-callable", expr->paren.line));
    } catch (std::bad_variant_access e) {
        throw (new RuntimeError("Illegal use of call operator on non-callable identifier", expr->paren.line));
    }
    
}

std::any Interpreter::visitBinaryExpr(Binary* expr) {
    std::any right = expr->right->accept(this);
    std::any left = expr->left->accept(this);
    switch (expr->op.type) {            
        case AND:
            return isTruthy(right) && isTruthy(left);
        case OR:
            return isTruthy(right) || isTruthy(left);
        case PLUS:
            if (left.type() == typeid(double) && right.type() == typeid(double)) {
                return std::any_cast<double>(left) + std::any_cast<double>(right);
            } else if (left.type() == typeid(std::string) && right.type() == typeid(std::string)) {
                return std::any_cast<std::string>(left) + std::any_cast<std::string>(right);
            } else {
                throw new CastError(0, "", "addition of invalid types");
            }
        case MINUS:
            castValid<double>(2, left, right);
            return any_cast<double>(left) - any_cast<double>(right);
        case SLASH:
            castValid<double>(2, left, right);
            return any_cast<double>(left) / any_cast<double>(right);
        case STAR:
            castValid<double>(2, left, right);
            return any_cast<double>(left) * any_cast<double>(right);
        case LESS:
            castValid<double>(2, left, right);
            return any_cast<double>(left) < any_cast<double>(right);
        case GREATER:
            castValid<double>(2, left, right);
            return any_cast<double>(left) > any_cast<double>(right);
        case GR_EQUAL:
            castValid<double>(2, left, right);
            return any_cast<double>(left) >= any_cast<double>(right);
        case LE_EQUAL:
            castValid<double>(2, left, right);
            return any_cast<double>(left) <= any_cast<double>(right);
        case IS_EQUAL:
            if(typeid(left) == typeid(right)){
                return huff::anyToString(left) == huff::anyToString(right);
            } else {
                return false;
            }
        case ISN_EQUAL:
            if(typeid(left) == typeid(right)){
                return huff::anyToString(left) != huff::anyToString(right);
            } else {
                return true;
            }
    }

    return NULL;
}

std::any Interpreter::visitAssignmentExpr(Assignment* expr) {
    std::any val = expr->expression->accept(this);
    env->assign(expr->name, val);
    return val;
}

std::any Interpreter::visitVariableExpr(Variable* var) {
    //Return map value for var token name (LEX)
    return env->pull(var->name);
}

bool Interpreter::isTruthy(std::any expr) {
    if (expr.has_value()){
        if (expr.type() == typeid(bool)){
            return any_cast<bool>(expr);
        }

        return true;
    }

    return false;
}

std::any Interpreter::executeBlock(Block* block, Enviroment* blockEnv) {
    Enviroment* prev = env;
    env = blockEnv;


    for (auto e: block->statements){
        if (typeid(*e) == typeid(Return)) {
            if (env->isFunc) {
                throw new NestedReturn(e->accept(this));
            }
        }
        
        e->accept(this);
    }
    env = prev;
    //delete blockEnv;

    return std::any();
}

template<typename T> void Interpreter::castValid(int c, ...) {
    va_list v;
    va_start(v, c);

    for (int x=0; x<c; x++){
        std::any arg = va_arg(v,std::any);
        if (arg.type() != typeid(T)){
            throw new CastError(0, arg.type().name(), "of type ");
        }
    }
}

void Interpreter::interpret(std::vector<Stmt*> stmts) {
    try {
        for (auto AST: stmts){
            AST->accept(this);
        }
    } catch (Err* error) {
        error->msg();
    }
}
