#pragma once

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <ctype.h>
#include "error.hpp"
#include "token.hpp"
#include "types.hpp"

  
class Scanner {

	private:

	std::vector<Token> tokens;
	std::string src;
	int line;
	int curr;
	int start;
	bool err = false;
	
	std::map<std::string,TokenType> keywords;

	public:
	Scanner(std::string src) {
		this->src = src;
		this->line = 1;
		this->curr = 0;

		keywords["and"] = AND;
		keywords["or"] = OR;
		keywords["not"] = NOT;
		keywords["while"] = WHILE;
		keywords["[+]"] = UDV;
		keywords["[~]"] = ARR;
		keywords["out"] = PRINT;
		keywords["class"] = CLASS;
		keywords["for"] = FOR;
		keywords["if"] = IF;
		keywords["elf"] = ELF;
		keywords["else"] = ELSE;
		keywords["switch"] = SWITCH;
		keywords["nul"] = NUL;
		keywords["false"] = FALSE;
		keywords["true"] = TRUE;
		keywords["return"] = RETURN;
		keywords["func"] = FUNC;
	}

	std::vector<Token> scan() {
		while (!atEnd()) {
			start = curr;
			evalToken();
		}
		return tokens;
	}

	void evalToken() {
		char c = forward();
		switch (c) {
			//Literals
			case '"': handleString(); break;

			//Brackets
			case '(': addToken(LEFT_BR);break;
			case ')': addToken(RIGHT_BR);break;
			case '{': addToken(LEFT_CURL);break;
			case '}': addToken(RIGHT_CURL);break;
			case ']': addToken(LEFT_SQ);break;
			case '[': addToken(RIGHT_SQ);break;
			case ';': addToken(SEMI_COL);break;

			//Connectors
			case '.': addToken(DOT);break;
			case ',': addToken(COMMA);break;

			//HEAD
			case '@':  addToken(AT);break;

			//operators
			case '*': addToken(STAR);break;
			case '+': addToken(PLUS);break;
			case '-': addToken(MINUS);break;
			case '=': addToken(match('=') ? IS_EQUAL : EQUAL); break;
			case '!': addToken(match('=') ? ISN_EQUAL : EXL); break;
			case '>': addToken(match('=') ? GR_EQUAL : GREATER); break;
			case '<': addToken(match('=') ? LE_EQUAL : LESS); break;
			case '/':if (match('/')){while(next()!='\n' && !atEnd()){forward();}}else{addToken(SLASH);}break;

			//other
			case '\n':line++;break;
			case '\r':break;
			case ' ':break;
			default:
				if (isdigit(c)){
					handleInt();
				} else if (isalpha(c)) {
					handleId();
				} else {
					throw(new UnexpectedSequence(line,src.substr(start,curr-start)));
				}
		}
	}


	void addToken(TokenType t){
		addToken(t,std::any());
	}

	void addToken(TokenType t, std::any literal){
		std::string lex = src.substr(start,curr-start);
		tokens.push_back(Token(t,lex,literal,line));
	}

	char forward(){
		return src.at(curr++);
	}

	bool match(char c){
		if (next()!=c){
			return false;
		} else {
			curr++;
			return true;
		}
	}

	bool atEnd(){
		return curr>=src.length();
	}

	char next(){
		if (!atEnd()){
			return src.at(curr);
		} else {
			return '\0';
		}
	}

	char view(int count) {
		if (count>=src.length()){
			return '\0';
		}
		return src.at(curr+count);
	}

	void handleString(){
		std::string lit;

		while (next() != '"'){
			if (atEnd()){
				throw(new UnexpectedSequence(line,src.substr(start,curr-start)));
			}
			lit += forward();
		}
		forward();

		addToken(STRING, std::string(lit));
	}

	void handleInt(){
		while (isdigit(next())){
			forward();
		}

		if (next() == '.' && isdigit(view(1))){
			forward();
			while (isdigit(next())){
				forward();
			}
		}
		addToken(INTEGER, double(stod(src.substr(start,curr-start))));
	}

	void handleId() {
		while (isalpha(next()) || isdigit(next()) || next()=='_'){
			forward();
		} 

		TokenType t = NUL;
		if (keywords.find(src.substr(start,curr-start)) == keywords.end()){
			t = IDENTIFIER;
		} else {
			t = keywords[src.substr(start,curr-start)];
		}

		addToken(t);
	}
};
