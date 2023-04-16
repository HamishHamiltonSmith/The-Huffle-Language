#pragma once

#include <iostream>
#include "token.hpp"

class Err {
    public:
    int line;
    virtual void msg() = 0;
};

class UnexpectedSequence : public Err {  
    std::string literal;

    public:
    UnexpectedSequence(int line,std::string literal) {
        this->line = line;
        this->literal = literal;
    }

    void msg() {
        std::cout << "\033[1;31;43m[HUFFL]\033[0m \033[31m Invalid sequence:\033[32m " << literal << "\033[0m on line " << line << "\n\n";
    }
};

class CastError : public Err {  
    std::string arg;
    std::string m;

    public:
    CastError(int line, std::string arg,std::string m) {
        this->line = line;
        this->arg = arg;
        this->m = m;
    }

    void msg() {
        std::cout << "\033[1;31;43m[HUFFL]\033[0m \033[31m Invalid cast:\033[32m " << m << "\033[0m" << arg << "\033[0m on line " << line << "\n\n";
    }
};

class RuntimeError : public Err {
    std::string m;

    public:
    RuntimeError(std::string m, int line) {
        this->m = m;
        this->line = line;

    }

    void msg() {
        std::cout << "\033[1;31;43m[HUFFL]\033[0m \033[31m Runtime Error:\033[32m " << m << "\033[0m on line " << line << "\n\n";
    }
};

class ParseError : public Err {
    std::string m;

    public:
    ParseError(std::string m, int line) {
        this->m=m;
        this->line = line;
    }

    void msg() {
        std::cout <<  "\033[1;31;43m[HUFFL]\033[0m \033[31m Parse error:\033[32m " << this->m << "\033[0m on line " << line << "\n\n";
    }
};
