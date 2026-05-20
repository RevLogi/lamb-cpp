#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <string_view>
#include <vector>

enum class TokenKind {
    OpenParen,
    CloseParen,
    Dot,
    Lambda,
    Name,
    End,
    OpenBrack,
    CloseBrack,
    OpenAngle,
    CloseAngle,
};

struct Token {
    TokenKind kind;
    std::string name;
};

class Lexer {
public:
    std::string_view source;
    size_t curr;

    Lexer(std::string_view source);

    Token scanToken();
    bool isEnd();

private:
    char peekAndAdvance();
    char peek();
    void skipWhiteSpaces();
    bool validNameChar(char& c);
};

std::vector<Token> tokenizer(std::string_view source);
void print_tokens(std::vector<Token>& tokens);

#endif
