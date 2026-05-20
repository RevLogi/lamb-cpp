#include "eval.hpp"

#include <iostream>

std::unordered_map<std::string, ExprPrt> global_env;

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
            } else if constexpr (std::is_same_v<T, Pair>) {
                std::cout << '[';
                display(node.left);
                std::cout << ' ';
                display(node.right);
                std::cout << ']';
            }
        },
        expr->data);
}

void test_trace(ExprPrt expr) {
    display(expr);
    std::cout << std::endl;
}

void trace_expr(ExprPrt expr) {
    display(expr);
    std::cout.flush();
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
                if (node.arg == arg && node.arg_id == id) {
                    return body;
                }
                ExprPrt new_body = apply(arg, node.body, val, id);
                return make_fun_naive(node.arg, node.arg_id, new_body);
            } else if constexpr (std::is_same_v<T, App>) {
                return make_app(apply(arg, node.lhs, val, id), apply(arg, node.rhs, val, id));
            } else if constexpr (std::is_same_v<T, Pair>) {
                return make_pair(apply(arg, node.left, val, id), apply(arg, node.right, val, id));
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
            } else if constexpr (std::is_same_v<T, Pair>) {
                ExprPrt left = eval(node.left);
                if (left != node.left) {
                    return make_pair(left, node.right);
                }

                ExprPrt right = eval(node.right);
                if (right != node.right) {
                    return make_pair(node.left, right);
                }

                return expr;
            }
        },
        expr->data);
}
