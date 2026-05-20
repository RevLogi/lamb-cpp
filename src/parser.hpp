#ifndef PARSER_HPP
#define PARSER_HPP

#include <stdexcept>
#include <string>
#include <vector>

#include "ast.hpp"
#include "lexer.hpp"

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& message) : std::runtime_error(message) {}
};

class Parser {
public:
    std::vector<Token> tokens;
    size_t curr;

    Parser(std::vector<Token>& tokens);

    bool peek(TokenKind kind);
    bool checkAndConsume(TokenKind kind);
    Token consumeAndReturn(TokenKind kind, std::string err_msg);
    Token peekAndAdvance();

private:
    bool isEnd();
};

ExprPrt parse_expr(Parser& parser);
ExprPrt parse_term(Parser& parser);

#endif
