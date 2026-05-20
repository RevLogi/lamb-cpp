#include "lexer.hpp"

#include <cctype>
#include <iostream>

Lexer::Lexer(std::string_view source) : source{source}, curr{0} {}

Token Lexer::scanToken() {
    skipWhiteSpaces();
    char c = peekAndAdvance();
    switch (c) {
        case '(':
            return {TokenKind::OpenParen, "("};
        case ')':
            return {TokenKind::CloseParen, ")"};
        case '.':
            return {TokenKind::Dot, "."};
        case '\\':
            return {TokenKind::Lambda, "\\"};
        case '[':
            return {TokenKind::OpenBrack, "["};
        case ']':
            return {TokenKind::CloseBrack, "]"};
        case '<':
            return {TokenKind::OpenAngle, "<"};
        case '>':
            return {TokenKind::CloseAngle, ">"};
        default:
            if (validNameChar(c)) {
                std::string name;
                name += c;
                char p = peek();
                while (!isEnd() && validNameChar(p)) {
                    name += peekAndAdvance();
                    p = peek();
                }
                return {TokenKind::Name, name};
            }

            return {TokenKind::End, ""};
    }
}

bool Lexer::isEnd() { return (curr >= source.size()); }

char Lexer::peekAndAdvance() {
    if (isEnd()) return '\0';
    return source.at(curr++);
}

char Lexer::peek() {
    if (isEnd()) return '\0';
    return source.at(curr);
}

void Lexer::skipWhiteSpaces() {
    while (!isEnd() && source.at(curr) == ' ') {
        curr++;
    }
}

bool Lexer::validNameChar(char& c) { return isalnum(c) || c == '-' || c == '_'; }

std::vector<Token> tokenizer(std::string_view source) {
    std::vector<Token> tokens;
    Lexer lexer(source);
    while (true) {
        Token new_token = lexer.scanToken();
        tokens.push_back(new_token);
        if (new_token.kind == TokenKind::End) break;
    }
    return tokens;
}

void print_tokens(std::vector<Token>& tokens) {
    for (Token token : tokens) {
        std::string token_kind;
        switch (token.kind) {
            case TokenKind::OpenParen:
                token_kind = "OpenParen";
                break;
            case TokenKind::CloseParen:
                token_kind = "CloseParen";
                break;
            case TokenKind::Dot:
                token_kind = "Dot";
                break;
            case TokenKind::Lambda:
                token_kind = "Lambda";
                break;
            case TokenKind::Name:
                token_kind = "Name";
                break;
            case TokenKind::End:
                token_kind = "End";
                break;
            case TokenKind::OpenBrack:
                token_kind = "OpenBrack";
                break;
            case TokenKind::CloseBrack:
                token_kind = "CloseBrack";
                break;
            case TokenKind::OpenAngle:
                token_kind = "OpenAngle";
                break;
            case TokenKind::CloseAngle:
                token_kind = "CloseAngle";
                break;
        }
        std::cout << token_kind << std::endl;
    }
}
