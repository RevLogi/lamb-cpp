#include "parser.hpp"

#include <vector>

#include "ast.hpp"
#include "lexer.hpp"

Parser::Parser(std::vector<Token>& tokens) : tokens{tokens}, curr{0} {}

bool Parser::peek(TokenKind kind) {
    TokenKind expected = isEnd() ? TokenKind::End : tokens[curr].kind;
    return kind == expected;
}

bool Parser::checkAndConsume(TokenKind kind) {
    TokenKind expected = isEnd() ? TokenKind::End : tokens[curr].kind;
    if (kind == expected) {
        curr++;
        return true;
    }
    return false;
}

Token Parser::consumeAndReturn(TokenKind kind, std::string err_msg) {
    TokenKind expected = isEnd() ? TokenKind::End : tokens[curr].kind;
    if (kind == expected) {
        return peekAndAdvance();
    }
    throw ParseError(err_msg);
}

Token Parser::peekAndAdvance() { return tokens[curr++]; }
bool Parser::isEnd() { return curr >= tokens.size(); }

ExprPrt parse_expr(Parser& parser) {
    ExprPrt expr = parse_term(parser);
    while (!(parser.peek(TokenKind::CloseParen) || parser.peek(TokenKind::CloseBrack) ||
             parser.peek(TokenKind::Dot) || parser.peek(TokenKind::End))) {
        ExprPrt rhs = parse_term(parser);
        expr = make_app(expr, rhs);
    }
    return expr;
}

ExprPrt parse_term(Parser& parser) {
    if (parser.checkAndConsume(TokenKind::Lambda)) {
        bool angleBracketAround = parser.peek(TokenKind::OpenAngle) ? true : false;
        if (angleBracketAround) {
            parser.consumeAndReturn(TokenKind::OpenAngle, "Function Name Syntax Error");
        }
        Token name_token = parser.consumeAndReturn(TokenKind::Name, "Function Name Syntax Error");
        std::vector<std::string> args;
        args.push_back(name_token.name);
        while (parser.peek(TokenKind::Name)) {
            args.push_back(parser.peekAndAdvance().name);
        }
        if (angleBracketAround) {
            parser.consumeAndReturn(TokenKind::CloseAngle, "Function Name Syntax Error");
        }

        parser.consumeAndReturn(TokenKind::Dot, "Function Dot Syntax Error");

        ExprPrt body;
        if (parser.peek(TokenKind::OpenBrack)) {
            parser.consumeAndReturn(TokenKind::OpenBrack, "Function Name Syntax Error");
            body = parse_expr(parser);
            parser.consumeAndReturn(TokenKind::CloseBrack, "Function Name Syntax Error");
        } else {
            body = parse_expr(parser);
        }

        ExprPrt fun = body;
        while (!args.empty()) {
            fun = make_fun(args.back(), fun);
            args.pop_back();
        }
        return fun;
    } else if (parser.checkAndConsume(TokenKind::OpenParen)) {
        ExprPrt res = parse_expr(parser);
        if (parser.checkAndConsume(TokenKind::Dot)) {
            ExprPrt right = parse_term(parser);
            if (parser.peek(TokenKind::OpenParen)) {
                throw ParseError("Pair Syntax Error");
            }
            res = make_pair(res, right);
            parser.consumeAndReturn(TokenKind::CloseParen, "Pair Syntax Error");
        } else {
            parser.consumeAndReturn(TokenKind::CloseParen, "Syntax Error");
        }
        return res;
    } else {
        Token name_token = parser.consumeAndReturn(TokenKind::Name, "Variable Syntax Error");
        std::string name = name_token.name;

        return make_var(name);
    }
}
