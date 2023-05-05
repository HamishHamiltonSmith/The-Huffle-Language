#pragma once
#include<iostream>
#include<vector>
#include<any>
#include "expr.hpp"
#include "utils.hpp"

class ExprVisitor;
class StmtVisitor;

//Mathematical
//sin .cos, tan, asin, acos, atan, PI, Eulers num, log, ln, pow, mod
//toBool, isNum, isStr, isDigit

class Interpreter : public ExprVisitor, public StmtVisitor {
    public:

    Enviroment* env;
    Enviroment* global;
    Interpreter();
    std::any visitPrintStmt(Print* stmt);
    std::any visitVarStmt(Var* stmt);
    std::any visitBlockStmt(Block* stmt);
    std::any visitConditionalStmt(Conditional* stmt);
    std::any visitCWhileStmt(CWhile* stmt);
    std::any visitFunctionStmt(Func* stmt);
    std::any visitExpressionStmt(Expression* stmt);
    std::any visitReturnStmt(Return* stmt);
    std::any visitLiteralExpr(Literal* expr);
    std::any visitGroupingExpr(Grouping* expr);
    std::any visitUnaryExpr(Unary* expr);
    std::any visitCallableExpr(Call* expr);
    std::any visitBinaryExpr(Binary* expr);
    std::any visitAssignmentExpr(Assignment* expr);
    std::any visitVariableExpr(Variable* var);
    std::any visitClassStmt(Class* stmt);
    bool isTruthy(std::any expr);
    std::any executeBlock(Block* block, Enviroment* blockEnv);
    template<typename T> void castValid(int c, ...);
    void interpret(std::vector<Stmt*> stmts);
};

struct HCallable {
    int numArgs;
    virtual std::any call(Interpreter* env, std::vector<std::any> args)=0;
};

class  NestedReturn {
    public:
    std::any val;

    NestedReturn(std::any v) {
        this->val = v;
    }
};

class UDCallable : public HCallable {
    public:
    int numArgs;
    Func* declaration;
    Enviroment* closure;
    UDCallable(Func* declaration, Enviroment* closure) {
        this->declaration = declaration;
        this->closure = closure;
    }

    std::any call(Interpreter* i, std::vector<std::any> args) {
        //Steps:

        //Currently every time a function is executed, we use the the current enviroment - this of course changes, and so function calls may work
        //in some situations, but not in ones where the enviroment is diferent to where it was located - so we will capture the enviroment where the function
        //is declared to ensure it runs consistently...

        Enviroment* funcEnv = new Enviroment(true, this->closure);
        if (args.size() == this->declaration->params.size()) {
            for (int x= 0; x<args.size(); x++) {
                funcEnv->define(this->declaration->params[x], args[x]);
            }
        } else { 
            throw new RuntimeError("Invalid argument count for function: " + this->declaration->name.lexeme,0); 
        }

        try {
            return i->executeBlock(new Block(this->declaration->body), funcEnv);
        } catch (NestedReturn* e) {
            return e->val;
        }  
        // Create new enviroment for funciton scope
        //Loop thorugh args and define in new enviroment - args are literals, use func body for names;
        //Execute block with new env
    }
};

class in : public HCallable {
    public:
    int numArgs=1;
    
    std::any call(Interpreter* i, std::vector<std::any> args) {
        std::cout << huff::anyToString(args[0]);
        std::string result;
        std::getline(std::cin, result);
        return result;
    }
}; 

class toNum : public HCallable {
    public:
    int numArgs=1;

    std::any call(Interpreter* i, std::vector<std::any> args) {
        try {
            return std::stod(std::any_cast<std::string>(args[0]));
        } catch (std::invalid_argument& e) {
            throw new RuntimeError("Can't convert to int - Invalid string",0);
        } catch (std::bad_any_cast& e) {
            try {
                return std::any_cast<double>(args[0]);
            } catch (std::bad_any_cast& e) {
                try {
                    return double(std::any_cast<bool>(args[0]));
                } catch (std::bad_any_cast& e) {
                    throw new RuntimeError("Unable to convert arg type to int",0);
                }
            }
        }
    }
};

class toStr : public HCallable {
    public:
    int numArgs=1;

    std::any call(Interpreter* i, std::vector<std::any> args) {
        try {
            return std::to_string(std::any_cast<double>(args[0]));
        } catch (std::invalid_argument& e) {
            throw new RuntimeError("Can't convert to string - Invalid arg",0);
        } catch (std::bad_any_cast& e) {
            try {
                return std::any_cast<std::string>(args[0]);
            } catch (std::bad_any_cast& e) {
                try {
                    return std::any_cast<bool>(args[0])==true ? true : false;
                } catch (std::bad_any_cast& e) {
                    throw new RuntimeError("Unable to convert arg type to string",0);
                }
            }
        }
    }
};

class leave : public HCallable {
    public:
    int numArgs=0;

    std::any call(Interpreter* i, std::vector<std::any> args) {
        exit(0);
    }
};

class length : public HCallable {
    public:
    int numArgs=1;

    std::any call(Interpreter* i, std::vector<std::any> args) {
        try {
            return (double)std::any_cast<std::string>(args[0]).length();
        } catch (std::bad_any_cast& e ) {
            throw new RuntimeError("Can't get length of non-string",0);
        }
    }
};

class type : public HCallable {
    public:
    int numArgs=1;

    std::any call(Interpreter* i, std::vector<std::any> args) {
        return std::string(args[0].type().name());
    }
};

class contains : public HCallable {
    public:
    int numArgs=2;

    std::any call(Interpreter* i, std::vector<std::any> args) {
        try {
            return std::any_cast<std::string>(args[0]).find(std::any_cast<std::string>(args[1])) != std::string::npos;
        } catch (std::bad_any_cast& e ) {
            throw new RuntimeError("Can't use contains() on non-string",0);
        }
    }
};
