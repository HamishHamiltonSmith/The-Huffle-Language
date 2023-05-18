#pragma once
#include<iostream>
#include<vector>
#include<any>
#include<math.h>
#include<thread>
#include<chrono>
#include "enviroment.hpp"
#include "page.hpp"
#include "expr.hpp"
#include "utils.hpp"

class ExprVisitor;
class StmtVisitor;

//Mathematical
//sin .cos, tan, asin, acos, atan, PI, Eulers num, log, ln, pow, mod
//toBool, isNum, isStr, isDigit

//Graphics integration
//page(hex colour, width, height) (str->hex resolver class)
//text(), rect(), polygon(), break(), divider()

//Polymorphic class, PageObject, Page:: vector<PageObjects>
//When a page is created, store _PAGE var in global enviroment for access when adding elements etc.
//PageObject->parentNode (PageObject) defines: position (attributes)



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
    virtual std::any call(Interpreter* env, Token& paren, std::vector<std::any> args)=0;
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

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
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
            throw new RuntimeError("Invalid argument count for function: " + this->declaration->name.lexeme,paren.line); 
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
    
    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        std::cout << huff::anyToString(args[0]);
        std::string result;
        std::getline(std::cin, result);
        return result;
    }
}; 

class toNum : public HCallable {
    public:
    int numArgs=1;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        try {
            return std::stod(std::any_cast<std::string>(args[0]));
        } catch (std::invalid_argument& e) {
            throw new RuntimeError("Can't convert to int - Invalid string",paren.line);
        } catch (std::bad_any_cast& e) {
            try {
                return std::any_cast<double>(args[0]);
            } catch (std::bad_any_cast& e) {
                try {
                    return double(std::any_cast<bool>(args[0]));
                } catch (std::bad_any_cast& e) {
                    throw new RuntimeError("Unable to convert arg type to int",paren.line);
                }
            }
        }
    }
};

class toStr : public HCallable {
    public:
    int numArgs=1;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        try {
            return std::to_string(std::any_cast<double>(args[0]));
        } catch (std::invalid_argument& e) {
            throw new RuntimeError("Can't convert to string - Invalid arg",paren.line);
        } catch (std::bad_any_cast& e) {
            try {
                return std::any_cast<std::string>(args[0]);
            } catch (std::bad_any_cast& e) {
                try {
                    return std::any_cast<bool>(args[0])==true ? true : false;
                } catch (std::bad_any_cast& e) {
                    throw new RuntimeError("Unable to convert arg type to string",paren.line);
                }
            }
        }
    }
};

class leave : public HCallable {
    public:
    int numArgs=0;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        exit(0);
    }
};

class length : public HCallable {
    public:
    int numArgs=1;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        try {
            return (double)std::any_cast<std::string>(args[0]).length();
        } catch (std::bad_any_cast& e ) {
            throw new RuntimeError("Can't get length of non-string",paren.line);
        }
    }
};

class type : public HCallable {
    public:
    int numArgs=1;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        return std::string(args[0].type().name());
    }
};

class Contains : public HCallable {
    public:
    int numArgs=2;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        try {
            return std::any_cast<std::string>(args[0]).find(std::any_cast<std::string>(args[1])) != std::string::npos;
        } catch (std::bad_any_cast& e ) {
            throw new RuntimeError("Can't use contains() on non-string",paren.line);
        }
    }
};

class Sin : public HCallable {
    public:
    int numArgs = 1;
    
    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        return sin(huff::anyToDouble(args[0]));
    }
};

class Cos : public HCallable {
    public:
    int numArgs = 1;
    
    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        return cos(huff::anyToDouble(args[0]));
    }
};

class Tan : public HCallable {
    public:
    int numArgs = 1;
    
    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        return tan(huff::anyToDouble(args[0]));
    }
};

class Asin : public HCallable {
    public:
    int numArgs = 1;
    
    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        return asin(huff::anyToDouble(args[0]));
    }
};

class Acos : public HCallable {
    public:
    int numArgs = 1;
    
    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        return acos(huff::anyToDouble(args[0]));
    }
};

class Atan : public HCallable {
    public:
    int numArgs = 1;
    
    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        return atan(huff::anyToDouble(args[0]));
    }
};

class Log : public HCallable {
    public:
    int numArgs =1;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        return log(huff::anyToDouble(args[0]));
    }
};

class Round : public HCallable {
    public:
    int numArgs =1;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        return round(huff::anyToDouble(args[0]));
    }
};

class Floor : public HCallable {
    public:
    int numArgs =1;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        return floor(huff::anyToDouble(args[0]));
    }
};

class Ceil : public HCallable {
    public:
    int numArgs = 1;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        return ceil(huff::anyToDouble(args[0]));
    }
};

class Pow : public HCallable {
    public:
    int numArgs = 2;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        return pow(huff::anyToDouble(args[0]), huff::anyToDouble(args[1]));
    }
};

class Sqrt : public HCallable {
    public:
    int numArgs = 2;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        return pow(huff::anyToDouble(args[0]), 1/(huff::anyToDouble(args[1])));
    }
};


//SYSTEM

class Sleep : public HCallable {
    public:
    int numArgs=1;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        std::this_thread::sleep_for(std::chrono::milliseconds((int)floor(std::any_cast<double>(args[0]))));
        return std::any();
    }
};

//GRAPHICS

class CreatePage : public HCallable {
    public:
    int numArgs=0;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        return new Page();
    }
};

class UpdatePage : public HCallable {
    public:
    int numArgs=1;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        try {
            std::any_cast<Page*>(args[0])->update();
        } catch (std::bad_any_cast& err) {
            throw new RuntimeError("Can't update non-page type",paren.line);
        }
        return std::any();
    }
};

class AddText : public HCallable {
    public:
    int numArgs=5;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        try {
            Page* p = std::any_cast<Page*>(args[0]);
            p->addElement(new Text(std::any_cast<std::string>(args[1]),DEFAULT_FONT_PATH, std::any_cast<double>(args[2]), std::any_cast<double>(args[3]), std::any_cast<double>(args[4])));
        } catch (std::bad_any_cast& err) {
            throw new RuntimeError("Text: Invalid parameter types",paren.line);
        }

        return std::any();
        //Args: (page (Page*), string (str), x (int), y (int), size (int))
    }
};

class AddRect : public HCallable {
    public:
    int numArgs=8;

    std::any call(Interpreter*i, Token& paren, std::vector<std::any> args) {
        try {
            Page* p = std::any_cast<Page*>(args[0]);
            sf::Color c = huff::resolveColor(std::any_cast<double>(args[1]), std::any_cast<double>(args[2]),std::any_cast<double>(args[3]));
            p->addElement(new Rect(c, std::any_cast<double>(args[4]), std::any_cast<double>(args[5]), std::any_cast<double>(args[6]), std::any_cast<double>(args[7])));
        } catch (std::bad_any_cast& err) {
            throw new RuntimeError("Rect: Invalid parameter types",paren.line);
        }

        return std::any();
     
    }
};