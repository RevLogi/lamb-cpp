#ifndef EVAL_HPP
#define EVAL_HPP

#include <string>
#include <unordered_map>

#include "ast.hpp"

extern std::unordered_map<std::string, ExprPrt> global_env;

ExprPrt eval(ExprPrt expr);
ExprPrt apply(std::string arg, ExprPrt body, ExprPrt val, size_t id);
void display(ExprPrt expr);
void test_trace(ExprPrt expr);
void trace_expr(ExprPrt expr);

#endif
