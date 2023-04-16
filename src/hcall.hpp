#pragma once
#include<iostream>
#include<vector>
#include<any>
#include"utils.hpp"
#include "enviroment.hpp"

struct HCallable {
    int numArgs;
    std::vector<std::type_info> argTypes;
    virtual std::any call(Enviroment* env, std::vector<std::any> args)=0;
};


class in : public HCallable {
    public:
    int numArgs=1;
    
    std::any call(Enviroment* env, std::vector<std::any> args) {
        std::cout << huff::anyToString(args[0]);
        std::string result;
        std::getline(std::cin, result);
        return result;
    }
}; 

class toNum : public HCallable {
    public:
    int numArgs=1;

    std::any call(Enviroment* env, std::vector<std::any> args) {
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

    std::any call(Enviroment* env, std::vector<std::any> args) {
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
                    throw new RuntimeError("Unable to convert arg type to int",0);
                }
            }
        }
    }
};


class type : public HCallable {
    public:
    int numArgs=1;

    std::any call(Enviroment* env, std::vector<std::any> args) {
        return std::string(args[0].type().name());
    }
};




//class Int : public HCallable {
//    public:
//    int numArgs=1;
//
//    std::any call(Interpreter& c, std::vector<std::any> args) {
//        c.env.assign((std::stoi(huff::anyToString(args[0]))), 10);
//        return NULL;
//    }
//};
