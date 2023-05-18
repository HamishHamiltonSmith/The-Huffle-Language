#pragma once
#include<any>
#include<iostream>
#include<string>
#include<any>
#include<SFML/Graphics/Color.hpp>
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

    double anyToDouble(std::any arg) {
        if (!arg.has_value()) {
            return 0;
        }

        try {
            return std::any_cast<double>(arg);
        } catch (std::bad_any_cast err) {
            try {
                return std::stod(std::any_cast<std::string>(arg));
            } catch (std::bad_any_cast err) {
                try {
                    bool val = std::any_cast<bool>(arg);
                    return (val) ? 1.0 : 0.0;
                } catch (std::bad_any_cast err) {
                    return 0;
                }
            } catch (std::invalid_argument err) {
                return 0;
            }
        }
    }

    sf::Color resolveColor(double r, double g, double b) {
        return sf::Color(r,g,b);
    }
}