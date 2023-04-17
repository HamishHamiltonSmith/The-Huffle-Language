#pragma once

#include <cstdarg>
#include <sstream>
#include <string>
#include "utils.hpp"
#include "error.hpp"
#include "types.hpp"
#include "expr.hpp"
#include "enviroment.hpp"
#include "hcall.hpp"

void addGlobal(Enviroment& env, std::string name, HCallable* callable) {
    env.define(name, callable);
}

class Interpreter : public ExprVisitor, public StmtVisitor {
    public:

    Enviroment* env;
    Enviroment* global;

    Interpreter() {
        env = new Enviroment();
        global = env;

        addGlobal(*global, "in", new in());
        addGlobal(*global, "type", new type());
        addGlobal(*global, "toNum", new toNum());
        addGlobal(*global, "toStr", new toStr());
        addGlobal(*global, "time", new clock());
    }

    //Statement Interpretation
    std::any visitPrintStmt(Print* stmt) {
        std::cout << huff::anyToString(stmt->expression->accept(this)) << "\n";
        return NULL;
    }

    std::any visitVarStmt(Var* stmt) {
        //Store variable in eviroment map
        env->define(stmt->name, stmt->initialiser->accept(this));
        return NULL;
    }

    std::any visitBlockStmt(Block* stmt) {
        Enviroment* blockEnv = new Enviroment(env);
        executeBlock(stmt, blockEnv);
        return NULL;
    }

    std::any visitConditionalStmt(Conditional* stmt) {
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

    std::any visitCWhileStmt(CWhile* stmt) {
        while (isTruthy(stmt->condition->accept(this))){
            stmt->body->accept(this);
        }

        return NULL;
    }

    

    std::any visitExpressionStmt(Expression* stmt) {
        stmt->expression->accept(this);
        return NULL;
    }

    //Expression Interpretation
    std::any visitLiteralExpr(Literal* expr) {
        return expr->value;
    }

    std::any visitGroupingExpr(Grouping* expr) {
        return expr->value->accept(this);
    }

    std::any visitUnaryExpr(Unary* expr) {
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
    
    std::any visitCallableExpr(Call* expr) {
        std::any callee = expr->callee->accept(this);

        std::vector<std::any> args;
        for (auto arg: expr->args) {
            args.push_back(arg->accept(this));
        }

        try {
            HCallable* func = std::any_cast<HCallable*>(callee);
            //if (func->numArgs != args.size()) {
            //    throw(new RuntimeError("Invalid call arg count, expected " + std::to_string(func->numArgs), expr->paren.line));
            //}

            return func->call(env,args);
        } catch (std::bad_any_cast e) {
            throw(new RuntimeError("Illegal use of call operater on non-callable", expr->paren.line));
        } catch (std::bad_variant_access e) {
            throw (new RuntimeError("Illegal use of call operator on non-callable identifier", expr->paren.line));
        }
        
    }

    std::any visitBinaryExpr(Binary* expr) {
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

    std::any visitAssignmentExpr(Assignment* expr) {
        std::any val = expr->expression->accept(this);
        env->assign(expr->name, val);
        return val;
    }

    std::any visitVariableExpr(Variable* var) {
        //Return map value for var token name (LEX)
        return env->pull(var->name);
    }

    bool isTruthy(std::any expr) {
        if (expr.has_value()){
            if (expr.type() == typeid(bool)){
                return any_cast<bool>(expr);
            }

            return true;
        }

        return false;
    }

    void executeBlock(Block* block, Enviroment* blockEnv) {
        Enviroment* prev = env;
        env = blockEnv;
        for (auto e: block->statements){
            e->accept(this);
        }
        env = prev;
        delete blockEnv;
    }

    template<typename T> void castValid(int c, ...) {
        va_list v;
        va_start(v, c);

        for (int x=0; x<c; x++){
            std::any arg = va_arg(v,std::any);
            if (arg.type() != typeid(T)){
                throw new CastError(0, arg.type().name(), "of type ");
            }
        }
    }

    public:

    void interpret(std::vector<Stmt*> stmts) {
        try {
            for (auto AST: stmts){
                AST->accept(this);
            }
        } catch (Err* error) {
            error->msg();
        }
    }
};

class AstPrinter : public ExprVisitor {
    std::any visitBinaryExpr(Binary* expr) {
        return parenthesize(expr->op.lexeme,2,expr->left,expr->right);
    }

    std::any visitGroupingExpr(Grouping* expr) {
        return parenthesize("group", 1, expr->value);
    }

    std::any visitLiteralExpr(Literal* expr) {
        return expr->value;
    }

    std::any visitUnaryExpr(Unary* expr){
        return parenthesize(expr->op.lexeme, 1, expr->right);
    }

    std::string parenthesize(std::string str, int count, ...) {
        std::string s;
        s.append("(").append(str);
        va_list l;
        va_start(l,count);
        for (int i=0; i<count; i++){
            s.append(" ");
            Expr* expr = va_arg(l,Expr*);
            std::string t = typeid(expr->accept(this)).name();
            s.append(huff::anyToString(expr->accept(this)));
        }

        s.append(")");

        return s;
    }

    public:
    std::string print(Expr* expr) {
        return huff::anyToString(expr->accept(this));
    }
};

