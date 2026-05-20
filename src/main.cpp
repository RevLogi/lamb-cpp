#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "eval.hpp"
#include "lexer.hpp"
#include "parser.hpp"

bool define(std::string input, bool prompt) {
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

                global_env[name] = eval(val_expr);
                if (prompt) std::cout << "Defined: " << name << std::endl;
            } catch (const ParseError &e) {
                std::cerr << "Define Error: " << e.what() << '\n';
            }
        }
        return true;
    } else {
        return false;
    }
}

void runPrompt(bool debug) {
    std::string input;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, input)) break;

        if (input.empty()) continue;
        if (input == "exit" || input == "quit") break;

        if (define(input, true)) {
            continue;
        }

        try {
            std::vector<Token> tokens = tokenizer(input);
            Parser parser(tokens);
            ExprPrt expr = parse_expr(parser);

            ExprPrt next = eval(expr);
            while (next != expr) {
                expr = next;
                if (debug) {
                    trace_expr(next);
                    std::string cmd;
                    std::getline(std::cin, cmd);
                }
                next = eval(expr);
            }
            if (!debug || next == expr) {
                trace_expr(expr);
                std::cout << std::endl;
            }
        } catch (const ParseError &e) {
            std::cerr << "Parse Error: " << e.what() << '\n';
        }
    }
}

void runFile(std::string path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "Can't open the file." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (define(line, false)) continue;

        try {
            std::vector<Token> tokens = tokenizer(line);
            Parser parser(tokens);
            ExprPrt expr = parse_expr(parser);

            ExprPrt next = eval(expr);
            while (next != expr) {
                expr = next;
                next = eval(expr);
            }
            if (next == expr) {
                trace_expr(expr);
                std::cout << std::endl;
            }
        } catch (const ParseError &e) {
            std::cerr << "Parse Error: " << e.what() << '\n';
        }
    }
}

int main(int argc, char **argv) {
    std::vector<std::string> args(argv + 1, argv + argc);
    std::string script_path = "";
    bool debug = false;

    for (auto it = args.begin(); it != args.end(); it++) {
        if (*it == "-d") {
            debug = true;
        } else if (*it == "-e") {
            if (std::next(it) != args.end()) {
                ++it;
                runFile(*it);
            } else {
                std::cerr << "Error: -e flag requires a file path.\n";
                return 1;
            }
        } else {
            script_path = *it;
        }
    }

    if (!script_path.empty()) {
        runFile(script_path);
        return 0;
    }

    std::cout << "Welcome to Lamb (C++ Edition)\n";
    std::cout << "Type 'exit' or 'quit' to exit.\n";

    runPrompt(debug);
    return 0;
}
