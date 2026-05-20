#ifndef AST_HPP
#define AST_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <variant>

extern size_t unique_id;

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

struct Pair {
    ExprPrt left;
    ExprPrt right;
};

using ExprData = std::variant<Var, Fun, App, Pair>;

struct Expr {
    ExprData data;
    Expr(ExprData d) : data(std::move(d)) {};
};

ExprPrt make_var(std::string name);
ExprPrt make_fun_naive(std::string arg, size_t id, ExprPrt body);
ExprPrt make_fun(std::string arg, ExprPrt body);
ExprPrt make_app(ExprPrt lhs, ExprPrt rhs);
ExprPrt make_pair(ExprPrt left, ExprPrt right);
ExprPrt bind_vars(ExprPrt body, std::string& arg, size_t new_id);

#endif
