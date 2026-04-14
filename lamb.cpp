#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

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
    // ((\y.(\x.y)) x)
    ExprPrt expr = make_app(make_app(make_fun("y", make_fun("x", make_var("y"))), make_var("x")),
                            make_var("unused"));

    trace_expr(expr);
    expr = eval(expr);
    trace_expr(expr);
    expr = eval(expr);
    trace_expr(expr);

    return 0;
}
