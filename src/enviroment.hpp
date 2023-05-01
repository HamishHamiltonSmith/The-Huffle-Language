#pragma once

#include <map>
#include <variant>
#include "token.hpp"
#include "error.hpp"

class Enviroment {
    std::map<std::string, std::any> values;
    public:
    Enviroment* enclosing;
    bool isFunc;

    //Global constructor
    Enviroment(bool isFunc) {
        enclosing = nullptr;
        this->isFunc = isFunc;
        this->enclosing = nullptr;
    }
    //Local constructor
    Enviroment(bool isFunc, Enviroment* enclosing) {
        this->enclosing = enclosing;
        this->isFunc = isFunc;
    }


    //Enviroment operations
    void define(Token name, std::any val) {
        values[name.lexeme] = val;
    }

    void define(std::string lex, std::any val) {
        values[lex] = val;
    }

    void assign(Token name, std::any val) {
        if (values.find(name.lexeme) != values.end()) {
            values[name.lexeme] = val;
            return;
        } 

        if (enclosing != nullptr){
            enclosing->assign(name,val);
            return;
        } 


        throw (new RuntimeError("Failed to find variable: " + name.lexeme, name.line));  
    }

    void assign(std::string lex, std::any val) {
        if (values.find(lex) != values.end()) {
            values[lex] = val;
            return;
        } 

        if (enclosing != nullptr){
            enclosing->assign(lex,val);
            return;
        } 


        throw (new RuntimeError("Failed to find variable: " + lex, 0));  
    }

    std::any pull(Token name) {
        if (values.find(name.lexeme) != values.end()) {
            return values[name.lexeme];
        }

        //Recursively traverse denested enviroments
        if (enclosing != nullptr){
            return enclosing->pull(name);
        } 

        throw (new RuntimeError("Failed to find variable: " + name.lexeme, name.line));   
    }
};
