#include <cctype>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

static size_t unique_id = 0;

struct Expr;
using ExprPrt = std::shared_ptr<Expr>;

struct Var {
    std::string name;
    size_t id;
};

struct Fun {
    std::string arg;
    size_t arg_id;
    ExprPrt body;
};

struct App {
    ExprPrt lhs;
    ExprPrt rhs;
};

using ExprData = std::variant<Var, Fun, App>;

struct Expr {
    ExprData data;
    Expr(ExprData d) : data(std::move(d)) {};
};

ExprPrt bind_vars(ExprPrt body, std::string &arg, size_t new_id);

ExprPrt make_var(std::string name) { return std::make_shared<Expr>(Var{std::move(name), 0}); }

ExprPrt make_fun_naive(std::string arg, size_t id, ExprPrt body) {
    return std::make_shared<Expr>(Fun{std::move(arg), id, body});
}

ExprPrt make_fun(std::string arg, ExprPrt body) {
    unique_id++;
    ExprPrt new_body = bind_vars(body, arg, unique_id);
    return std::make_shared<Expr>(Fun{std::move(arg), unique_id, new_body});
}

ExprPrt make_app(ExprPrt lhs, ExprPrt rhs) { return std::make_shared<Expr>(App{lhs, rhs}); }

enum class TokenKind { OpenParen, CloseParen, Dot, Lambda, Name, End };

struct Token {
    TokenKind kind;
    std::string name;
};

class Lexer {
public:
    std::string_view source;
    size_t curr;

    Lexer(std::string_view source) : source{source}, curr{0} {}

    Token scanToken() {
        skipWhiteSpaces();
        char c = advance();
        switch (c) {
            case '(':
                return {TokenKind::OpenParen, "("};
            case ')':
                return {TokenKind::CloseParen, ")"};
            case '.':
                return {TokenKind::Dot, "."};
            case '\\':
                return {TokenKind::Lambda, "\\"};
            default:
                if (isalnum(c)) {
                    std::string name;
                    name += c;
                    while (!isEnd() && isalnum(peek())) {
                        name += advance();
                    }
                    return {TokenKind::Name, name};
                }

                return {TokenKind::End, ""};
        }
    }

    bool isEnd() { return (curr >= source.size()); }

private:
    char advance() {
        if (isEnd()) return '\0';
        return source.at(curr++);
    }

    char peek() {
        if (isEnd()) return '\0';
        return source.at(curr);
    }

    void skipWhiteSpaces() {
        while (!isEnd() && source.at(curr) == ' ') {
            curr++;
        }
    }
};

std::vector<Token> tokenizer(std::string_view source) {
    std::vector<Token> tokens;
    Lexer lexer(source);
    while (!lexer.isEnd()) {
        Token new_token = lexer.scanToken();
        if (new_token.kind == TokenKind::End) break;
        tokens.push_back(new_token);
    }
    return tokens;
}

void pparse(std::string source) {
    std::vector<Token> tokens = tokenizer(source);
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
        }
        std::cout << token_kind << std::endl;
    }
}

/** Bind all the vars in the body (recursively)
 *  that has the same name of the outer arg with the new_id (unique) **/
ExprPrt bind_vars(ExprPrt body, std::string &arg, size_t new_id) {
    return std::visit(
        [&](const auto &node) -> ExprPrt {
            using T = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<T, Var>) {
                if (node.name == arg) {
                    return std::make_shared<Expr>(Var{node.name, new_id});
                }
                return body;
            } else if constexpr (std::is_same_v<T, Fun>) {
                // Enable shadowing
                // Allow the var stay free and be bound by the inner function
                if (node.arg == arg) {
                    return body;
                }
                ExprPrt new_body = bind_vars(node.body, arg, new_id);
                return make_fun_naive(node.arg, node.arg_id, new_body);
            } else if constexpr (std::is_same_v<T, App>) {
                return make_app(bind_vars(node.lhs, arg, new_id), bind_vars(node.rhs, arg, new_id));
            }
        },
        body->data);
}

void display(ExprPrt expr) {
    std::visit(
        [](const auto &node) {
            using T = std::decay_t<decltype(node)>;

            if constexpr (std::is_same_v<T, Var>) {
                std::cout << node.name << '_' << node.id;
            } else if constexpr (std::is_same_v<T, Fun>) {
                std::cout << "(\\" << node.arg << '_' << node.arg_id << '.';
                display(node.body);
                std::cout << ")";
            } else if constexpr (std::is_same_v<T, App>) {
                std::cout << '(';
                display(node.lhs);
                std::cout << ' ';
                display(node.rhs);
                std::cout << ')';
            }
        },
        expr->data);
}

void trace_expr(ExprPrt expr) {
    display(expr);
    std::cout << std::endl;
}

/** Apply val to the body (recursively)
 *  Return the new body **/
ExprPrt apply(std::string arg, ExprPrt body, ExprPrt val, size_t id) {
    return std::visit(
        [&](const auto &node) -> ExprPrt {
            using T = std::decay_t<decltype(node)>;

            if constexpr (std::is_same_v<T, Var>) {
                if (node.name == arg && node.id == id) {
                    return val;
                }
                return body;
            } else if constexpr (std::is_same_v<T, Fun>) {
                ExprPrt new_body = apply(arg, node.body, val, id);
                // Keep the old id
                return make_fun_naive(node.arg, node.arg_id, new_body);
            } else if constexpr (std::is_same_v<T, App>) {
                return make_app(apply(arg, node.lhs, val, id), apply(arg, node.rhs, val, id));
            }
        },
        body->data);
}

ExprPrt eval(ExprPrt expr) {
    return std::visit(
        [&](const auto &node) -> ExprPrt {
            using T = std::decay_t<decltype(node)>;

            if constexpr (std::is_same_v<T, Var>) {
                return expr;
            } else if constexpr (std::is_same_v<T, Fun>) {
                ExprPrt new_body = eval(node.body);
                if (new_body != node.body) {
                    return make_fun_naive(node.arg, node.arg_id, new_body);
                }
                return expr;
            } else if constexpr (std::is_same_v<T, App>) {
                ExprPrt lhs = eval(node.lhs);

                if (lhs != node.lhs) {
                    return make_app(lhs, node.rhs);
                }

                if (std::holds_alternative<Fun>(lhs->data)) {
                    auto &fun_node = std::get<Fun>(lhs->data);
                    return apply(fun_node.arg, fun_node.body, node.rhs, fun_node.arg_id);
                }

                ExprPrt rhs = eval(node.rhs);
                if (rhs != node.rhs) {
                    return make_app(lhs, rhs);
                }

                return expr;
            }
        },
        expr->data);
}

int main() {
    // (\x.((\y.y) x))
    // ExprPrt expr = make_fun("x", make_app(make_fun("y", make_var("y")), make_var("x")));
    // (((\y.(\x.y)) x) unused)
    std::string raw_expr = "(((\\y.(\\x.y)) x) unused)";
    pparse(raw_expr);
    ExprPrt expr = make_app(make_app(make_fun("y", make_fun("x", make_var("y"))), make_var("x")),
                            make_var("unused"));

    trace_expr(expr);
    expr = eval(expr);
    trace_expr(expr);
    expr = eval(expr);
    trace_expr(expr);

    return 0;
}
