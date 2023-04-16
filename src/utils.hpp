#pragma once
#include<any>
#include<iostream>
#include<string>
#include<any>
#include<stdarg.h>

namespace huff {
    std::string anyToString(std::any arg) {
        if (!arg.has_value()) {
            return "";
        }
        try {
            return std::any_cast<std::string>(arg);
        } catch (std::bad_any_cast err) {
            try {
                return std::to_string(std::any_cast<int>(arg));
            } catch (std::bad_any_cast err) {
                try {
                    return std::to_string(std::any_cast<double>(arg));
                } catch (std::bad_any_cast err) {
                    try {
                        return std::to_string(std::any_cast<bool>(arg))=="1" ? "true" : "false";
                    } catch (std::bad_any_cast err){
                        return "";
                    }
                }
            }
        }
    }
}