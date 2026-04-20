#include <cctype>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
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

std::unordered_map<std::string, ExprPrt> global_env;

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
                if (validNameChar(c)) {
                    std::string name;
                    name += c;
                    char p = peek();
                    while (!isEnd() && validNameChar(p)) {
                        name += advance();
                        p = peek();
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

    bool validNameChar(char &c) { return isalnum(c) || c == '-' || c == '_'; }
};

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string &message) : std::runtime_error(message) {}
};

class Parser {
public:
    std::vector<Token> tokens;
    size_t curr = 0;

    Parser(std::vector<Token> &tokens) : tokens{tokens} {}

    bool peek(TokenKind kind) {
        TokenKind expected = isEnd() ? TokenKind::End : tokens[curr].kind;
        return kind == expected;
    }

    bool consume(TokenKind kind) {
        TokenKind expected = isEnd() ? TokenKind::End : tokens[curr].kind;
        if (kind == expected) {
            curr++;
            return true;
        }
        return false;
    }

    Token consume(TokenKind kind, std::string err_msg) {
        TokenKind expected = isEnd() ? TokenKind::End : tokens[curr].kind;
        if (kind == expected) {
            return advance();
        }
        throw ParseError(err_msg);
    }

private:
    Token advance() { return tokens[curr++]; }
    bool isEnd() { return curr >= tokens.size(); }
};

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

ExprPrt parse_fun(Parser &parser);
ExprPrt parse_var(Parser &parser);
ExprPrt parse_term(Parser &parser);

ExprPrt parse_expr(Parser &parser) {
    ExprPrt expr = parse_term(parser);
    // Only peek, not consume
    while (!(parser.peek(TokenKind::CloseParen) || parser.peek(TokenKind::End))) {
        ExprPrt rhs = parse_term(parser);
        expr = make_app(expr, rhs);
    }
    return expr;
}

ExprPrt parse_term(Parser &parser) {
    if (parser.consume(TokenKind::Lambda)) {
        return parse_fun(parser);
    } else if (parser.consume(TokenKind::OpenParen)) {
        ExprPrt res = parse_expr(parser);
        parser.consume(TokenKind::CloseParen, "Syntax Error");
        return res;
    } else {
        return parse_var(parser);
    }
}

ExprPrt parse_fun(Parser &parser) {
    Token name_token = parser.consume(TokenKind::Name, "Function Name Syntax Error");
    std::string name = name_token.name;
    parser.consume(TokenKind::Dot, "Function Dot Syntax Error");
    ExprPrt body = parse_expr(parser);

    return make_fun(name, body);
}

ExprPrt parse_var(Parser &parser) {
    std::string err_msg = "Variable Syntax Error";

    Token name_token = parser.consume(TokenKind::Name, err_msg);
    std::string name = name_token.name;

    return make_var(name);
}

void pparse(std::vector<Token> &tokens) {
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
                std::cout << node.name;
            } else if constexpr (std::is_same_v<T, Fun>) {
                std::cout << "(\\" << node.arg << '.';
                if (std::holds_alternative<App>(node.body->data)) {
                    auto &app_node = std::get<App>(node.body->data);
                    display(app_node.lhs);
                    std::cout << ' ';
                    display(app_node.rhs);
                } else {
                    display(node.body);
                }
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
    std::cout.flush();
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
                if (node.arg == arg && node.arg_id == id) {
                    return body;
                }
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
                if (node.id == 0) {
                    auto it = global_env.find(node.name);
                    if (it != global_env.end()) {
                        return eval(it->second);
                    }
                }
                return expr;
            } else if constexpr (std::is_same_v<T, Fun>) {
                ExprPrt new_body = eval(node.body);
                if (new_body != node.body) {
                    return make_fun_naive(node.arg, node.arg_id, new_body);
                }
                return expr;
            } else if constexpr (std::is_same_v<T, App>) {
                if (std::holds_alternative<Fun>(node.lhs->data)) {
                    auto &fun_node = std::get<Fun>(node.lhs->data);
                    return apply(fun_node.arg, fun_node.body, node.rhs, fun_node.arg_id);
                }

                ExprPrt lhs = eval(node.lhs);
                if (lhs != node.lhs) {
                    return make_app(lhs, node.rhs);
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

int main(int argc, char **argv) {
    int debug = 0;
    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        debug = 1;
    }

    std::cout << "Welcome to Lamb (C++ Edition)\n";
    std::cout << "Type 'exit' or 'quit' to exit.\n";

    std::string input;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, input)) break;

        if (input.empty()) continue;
        if (input == "exit" || input == "quit") break;

        if (input.substr(0, 7) == "define ") {
            size_t eq_pos = input.find(" ", 7);
            if (eq_pos != std::string::npos) {
                std::string name = input.substr(7, eq_pos - 7);
                name.erase(0, name.find_first_not_of(" "));
                if (!name.empty()) {
                    name.erase(name.find_last_not_of(" ") + 1);
                }

                std::string expr_str = input.substr(eq_pos + 1);

                try {
                    std::vector<Token> tokens = tokenizer(expr_str);
                    Parser parser(tokens);
                    ExprPrt val_expr = parse_expr(parser);

                    global_env[name] = val_expr;
                    std::cout << "Defined: " << name << std::endl;
                } catch (const ParseError &e) {
                    std::cerr << "Define Error: " << e.what() << '\n';
                }
                continue;
            }
        }

        try {
            std::vector<Token> tokens = tokenizer(input);
            Parser parser(tokens);
            ExprPrt expr = parse_expr(parser);

            ExprPrt next = eval(expr);
            if (next == expr) {
                trace_expr(expr);
                std::cout << std::endl;
            }
            while (next != expr) {
                expr = next;
                if (debug) {
                    trace_expr(next);
                    std::string cmd;
                    std::getline(std::cin, cmd);
                }
                next = eval(expr);
            }
            if (!debug) {
                trace_expr(expr);
                std::cout << std::endl;
            }
        } catch (const ParseError &e) {
            std::cerr << "Parse Error: " << e.what() << '\n';
        }
    }
    return 0;

    // (\x.((\y.y) x))
    // ExprPrt expr = make_fun("x", make_app(make_fun("y", make_var("y")), make_var("x")));
    // (((\y.(\x.y)) x) unused)
}
