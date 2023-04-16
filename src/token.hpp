#pragma once

#include <string>
#include <iostream>
#include "types.hpp"
#include <any>



class Token {
	public:

	int line;
	TokenType type;
	std::string lexeme;
	std::any literal;
	

	public:
	Token() = default;
	
	Token(TokenType type, std::string lexeme, std::any literal, int line) {
		this->line = line;
		this->type = type;
		this->lexeme = lexeme;
		this->literal = literal;
	} 

	void outputToken(){
		std::cout << "TOKEN " << convert[type] << " RAW: " <<  lexeme << " ON LINE: " << line << std::endl;
	}
};
