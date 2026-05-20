#include "ast.hpp"

size_t unique_id = 0;

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

ExprPrt make_pair(ExprPrt left, ExprPrt right) { return std::make_shared<Expr>(Pair{left, right}); }

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
            } else if constexpr (std::is_same_v<T, Pair>) {
                return make_pair(bind_vars(node.left, arg, new_id),
                                 bind_vars(node.right, arg, new_id));
            }
        },
        body->data);
}
