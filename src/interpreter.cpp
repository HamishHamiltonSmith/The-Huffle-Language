
#include <iostream>
#include <string>
#include <sstream>
#include "parser.hpp"
#include "scanner.hpp"
#include "types.hpp"
#include "token.hpp"
#include "visitor.hpp"
#include "error.hpp"
#include <fstream>


bool hadErr = false;

void lrun(std::string l){
	Scanner p = Scanner(l);
	std::vector<Stmt*> e;
	try {
		
		std::vector<Token> tokens = p.scan();
		tokens.push_back(Token(EF,"",'\0',0));

		Parser p = Parser(tokens);
		e = p.parse();

		//AstPrinter printer = AstPrinter();
		//std::cout << printer.print(e) << std::endl;
	} catch (Err* err) {
		err->msg();
		hadErr = true;
	};	

	try {
	Interpreter eval = Interpreter();
	eval.interpret(e);
	} catch (Err* err) {
		err->msg();
	}
}

void runFile(char* path) {
	std::ifstream input = std::ifstream(path);

	std::stringstream buffer;
	buffer << input.rdbuf();
	lrun(buffer.str());
}

int main(int argc, char* argv[]) {
	if (argc == 2){
		 runFile(argv[1]);
	} else {
		std::cout << "Huff Usage: huffl [filename].huff" << std::endl;
	}
	return 0;
}
