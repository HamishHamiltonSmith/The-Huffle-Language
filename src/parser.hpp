#pragma once
#include <vector>
#include <cstdarg>
#include "token.hpp"
#include "types.hpp"
#include "expr.hpp"
#include "error.hpp"
#include "utils.hpp"




class Parser {
    std::vector<Token> tokens;
    int current = 0;

    bool isAtEnd() {
        return peek().type == EF;
    }

    Token peek() {
        return tokens.at(current);
    }

    Token previous() {
        return tokens.at(current-1);
    }

    bool check(TokenType t) {
        if (isAtEnd()){
            return false;
        } else if (tokens.at(current).type == t){
            return true;
        }

        return false;
    }

    void synchronise() {
        //Discards tokens until semi col or new expression reched
        advance();

        while (!isAtEnd()){

            if (previous().type == SEMI_COL) return;

            switch (peek().type) {
                case CLASS:
                case FUNC:
                case UDV:
                case FOR:
                case IF:
                case ELF:
                case ELSE:
                case WHILE:
                case PRINT:
                case RETURN:
                return;
            }
            advance();
        }
    }

    Token advance() {
        if (!isAtEnd()) current++;
        return previous();
    }

    bool match(int count, ...) {
        va_list v;
        va_start(v,count);

        for (int i=0; i<count; i++) {
            TokenType e = (TokenType)va_arg(v, int);
            if (check(e)){
                advance();
                return true;
            }
        }

        return false;
    }

    Token consume(TokenType t, std::string msg) {
        if (check(t)){
            return advance();
        } else {
            throw(new ParseError(msg, tokens.at(current-1).line));
        }
    }

    public:
    Parser(std::vector<Token>  tokens) {
        this->tokens = tokens;
    }

    std::vector<Stmt*> parse() {
        std::vector<Stmt*> stmts;

        try {
            while (!isAtEnd()){
                stmts.push_back(declaration());
            }
            return stmts;

        } catch (Err* e) {
            e->msg();
        }

        return std::vector<Stmt*>(0);
    }

    Stmt* declaration() {
        try {
            if (match(1,UDV)) {
                return varDeclaration();
            } else if (match(1, FUNC)) {
                return funcDeclaration("function");
            } else if (match(1, CLASS)) {
                return classDeclaration();
            }

            return statement();
        } catch (ParseError error) {
                synchronise();
                return NULL;
        }
    }

    Stmt* varDeclaration() {
        Token name = consume(IDENTIFIER, "expected indentifier after UDV statement");
        if (match(1,SEMI_COL)){
            //Create new variable
            return new Var(name,new Literal(std::any(double(0))));
        }

        consume(EQUAL, "invalid variable declaration (expected ';' or '='");
        Expr* val = expression();
        consume(SEMI_COL, "Expected semi-colon after statement");
        return new Var(name, val); 
    }

    Func* funcDeclaration(std::string type) {
        Token name = consume(IDENTIFIER, "expected identifier after" + type + "statement");
        Token bracket = consume(LEFT_BR, "Expected a '(' before" + type +  "parameter list");
        std::vector<Token> args;
        if (!check(RIGHT_BR)) {
            args.push_back(consume(IDENTIFIER, "Function parameters must only consist of identifiers"));
            while (match(1,COMMA)){
                args.push_back(consume(IDENTIFIER, "Function parameters must only consist of identifiers"));

                if (args.size() >= 255) {
                    throw ParseError("Function declaration has too many parameters", bracket.line);
                }
            }
        }
        consume(RIGHT_BR, "Expected a ')' after function parameter list");
        consume(LEFT_CURL, "Exprected block after function signature");
        std::vector<Stmt*> body = block();
        return new Func(name, args, body);
    }

    Stmt* classDeclaration() {
        Token name = consume(IDENTIFIER, "expected identifier after class statement");
        if (match(1,SEMI_COL)) {
            return new Class(name, std::vector<Func*>());
        }

        consume(LEFT_CURL, "expected ';' or '{' after class statement");
        std::vector<Func*> methods;
        while (!isAtEnd() && !check(RIGHT_CURL)) {
            methods.push_back(funcDeclaration("c_method"));
        }

        consume(RIGHT_CURL, "Expected '}' after function body");
        
        return new Class(name, methods);
    };

    Stmt* statement() {
        if (match(1,PRINT)) {
            consume(LEFT_BR,"expected a '(' before print statement");
            Stmt* exp = printStatement();
            return exp;
        } else if (match(1,LEFT_CURL)){
            return new Block(block());
        } else if (match(1, IF)) {
            return conditional();
        } else if (match(1, WHILE)){
            return whileLoop();
        } else if (match(1,FOR)){
            return forLoop();
        } else if (match(1, RETURN)){
            Expr* exp = expression();
            consume(SEMI_COL, "Expected semi-colon after return statement");
            return new Return(exp);
        }

        return  expressionStatement();
    }

    Stmt* expressionStatement() {
        Expr* val = expression();
        consume(SEMI_COL, "Expected semi-colon after statement");
        return new Expression(val);
    }

    Stmt* conditional() {
        consume(LEFT_BR, "Exprected '(' before conditional expression");
        Expr* condition = expression();
        consume(RIGHT_BR, "Expected ')' after conditional expression");
        Stmt* thenBranch = statement();

        if (match(1,ELSE)) {
            Stmt* elseBranch = statement();
            return new Conditional(condition,thenBranch,elseBranch);
        } else {
            std::vector<Conditional*> elfs;
            while (match(1,ELF) && !isAtEnd()) {
                elfs.push_back(elfConditional());
            }

            return new Conditional(condition,thenBranch,elfs);
        }
    }

    Stmt* whileLoop() {
        consume(LEFT_BR, "Expected '(' before loop expression");
        Expr* condition = expression();
        consume(RIGHT_BR, "Expected ')' after loop expression");
        Stmt* body = statement();

        return new CWhile(condition, body);
    }

    Stmt* forLoop() {
        consume(LEFT_BR, "Expected '(' after for keyword");

        Stmt* init;
        if (match(1,SEMI_COL)) {
            //no initialiser
            init = nullptr;
        } else if (match(1,UDV)) {
            init = varDeclaration();
        } else {
            init = expressionStatement();
        }

        Expr* condition = nullptr;

        if (!check(SEMI_COL)){
            condition =  expression();
        }

        consume(SEMI_COL, "Expected ';' after for condition");

        Stmt* increment = nullptr;
        if (!check(RIGHT_BR)) {
            increment = new Expression(expression());
        }
        consume(RIGHT_BR, "Expect ')' after for clauses.");

        Stmt* body = statement();

        if (increment != nullptr) {
            body = new Block(std::vector<Stmt*>{body,increment});
        }

        if (condition == nullptr) {
            condition=new Literal(true);
        }
        body = new CWhile(condition, body);

        if (init != nullptr) {
            body = new Block(std::vector<Stmt*>{init,body});

            //ie:
            //{
            //    udv x = 0;
            //    while (...){}
            //}
            ////rather than:
            //while(...){udv x=0}
        }

        return body;


    }

    Conditional* elfConditional() {
        consume(LEFT_BR, "Exprected '(' before conditional expression");
        Expr* condition = expression();
        consume(RIGHT_BR, "Expected ')' after conditional expression");
        Stmt* thenBranch = statement();

        if (match(1,ELSE)) {
            return new Conditional(condition, thenBranch, statement());
        } else {
            return new Conditional(condition,thenBranch);
        }
    }

    std::vector<Stmt*> block() {
        std::vector<Stmt*> stmts;
        while (!isAtEnd() && !check(RIGHT_CURL)){
            stmts.push_back(declaration());
        }

        consume(RIGHT_CURL, "Expected a '}' after scope");
        return stmts;
    }

    Stmt* printStatement() {
        Expr* val = expression();
        consume(RIGHT_BR,"expected a ') after print statement");
        consume(SEMI_COL, "Exprected semi-colon after statement");
        return new Print(val);
    }


    Expr* expression() {
        return assignment();
    }

    Expr* assignment() {
        Expr* val = logicalOr();

        if (match(1,EQUAL))  {
            try {
                Variable* lval = dynamic_cast<Variable*>(val);

                //Not 'equality' since x=y=10 is allowed (recursive assignment)
                Expr* right = assignment();
                return new Assignment(dynamic_cast<Variable*>(lval)->name, right);
            } catch (std::bad_cast err) {
                throw ParseError("Invalid assignment of non-udv", tokens.at(current-2).line);
            }
        }

        return val;
    }

    Expr* logicalOr() {
        Expr* expr = logicalAnd();
        while (match(1,OR)){
            Token op = previous();
            Expr* right = logicalAnd();
            expr = new Binary(expr, op, right);
        }

        return expr;
    }

    Expr* logicalAnd() {
        Expr* expr = equality();
        while (match(1,AND)){
            Token op = previous();
            Expr* right = equality();
            expr = new Binary(expr, op, right);
        }

        return expr;
    }

    Expr* equality() {
        Expr* expr = comparison();

        while (match(2, ISN_EQUAL, IS_EQUAL)) {
            Token op = previous();
            Expr* right = comparison();
            expr = new Binary(expr,op,right);
        }

        return expr;
    }

    Expr* comparison() {
        Expr* expr = term();
        while (match(4, LE_EQUAL, GR_EQUAL, GREATER, LESS)) {
            Token op = previous();
            Expr* right = term();
            expr = new Binary(expr,op,right);
        }

        return expr;
    }

    Expr* term() {
        Expr* expr = factor();
        while (match(2,MINUS,PLUS)) {
            Token op = previous();
            Expr* right = factor();
            expr = new Binary(expr,op,right);
        }

        return expr;
    
    }

    Expr* factor() {
        Expr* expr = unary();
        while (match(2,STAR,SLASH)) {
            Token op = previous();
            Expr* right = unary();
            expr = new Binary(expr,op,right);
        }

        return expr;
    }

    Expr* unary(){
        if (match(2,MINUS, EXL)) {
            Token op = previous();
            Expr* right = unary();
            return new Unary(op, right); 
        }

        return call();
    }

    Expr* call()  {
        Expr* expr = primary();
        while (true) {
            //Encountered call token
            if (match(1,LEFT_BR)) {
                expr = finishCall(expr);
            } else {
                break;
            }
        }

        return expr;
    }

    Expr* finishCall(Expr* expr) {
        std::vector<Expr*> v;

        if (!check(RIGHT_BR)) {
            v.push_back(expression());
            while (match(1, COMMA)){
                v.push_back(expression());
                if (v.size() >= 255) {
                    throw ParseError("Too many arguements for function call", tokens.at(current).line);
                }
            }
        }


        Token bracket = consume(RIGHT_BR, "Expected ')' in call expression");
        return new Call(expr, v, bracket);
    }

    Expr* primary() {
        if (match(1, FALSE)) {
            return new Literal(false);
        } else if (match(1,TRUE)) {
            return new Literal(true);
        } else if (match(1,IDENTIFIER)) {
            return new Variable(previous());
        }else if (match(1,NUL)) {
            return new Literal(NULL);
        } else if (match(2,INTEGER,STRING)) {
            return new Literal(previous().literal);
        } else if (match(1,LEFT_BR)){
            Expr* expr = expression();
            consume(RIGHT_BR, "Expected a ')' after grouped expression");
            return new Grouping(expr);
        } else {
            throw(new ParseError("Invalid token",tokens.at(current).line));
        }
    }

};
