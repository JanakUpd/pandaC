#include "lexer.h"

#include <iostream>
#include <string>
#include <vector>
#include <variant>

std::pair<float, float> Lexer::getBindingPower(const std::string &symbol) {
    if (symbol == "or") return {1.0, 1.1};
    if (symbol == "and") return {2.0, 2.1};
    if (symbol == "not") return {0.0, 3.1};
    if (symbol == "+") return {4.0, 4.1};
    if (symbol == "-") return {4.0, 4.1};
    if (symbol == "*") return {5.0, 5.1};
    if (symbol == "/") return {5.0, 5.1};
    if (symbol == "**") return {6.1, 6.0};
    if (symbol == "(") return {8.0, 0.0};
    if (symbol == ",") return {0.0, 0.0};
    if (symbol == ")") return {0.0, 0.0};
    if (symbol == "<") return {3.2, 3.3};
    if (symbol == ">") return {3.2, 3.3};
    if (symbol == "<=") return {3.2, 3.3};
    if (symbol == ">=") return {3.2, 3.3};
    if (symbol == "==") return {3.2, 3.3};
    if (symbol == "!=") return {3.2, 3.3};
    if (symbol == ".") return {9.0, 9.1};
    if (symbol == "[") return {8.0, 0.0};
    if (symbol == "]") return {0.0, 0.0};
    if (symbol == "=") return {0.1, 0.0};
    throw std::invalid_argument("invalid operator: " + symbol);
}

Token Lexer::peek() {
    if (current_token_index >= tokens.size()) {
        return Token{TokenType::Eof, ""};
    }
    return tokens[current_token_index];
}

Token Lexer::consume() {
    Token t = peek();
    ++current_token_index;
    return t;
}

void Lexer::replace(std::string &str, const std::string &from, const std::string &to) {
    if (from.empty()) return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

Expression Lexer::fromString(std::string input) {
    tokens.clear();
    current_token_index = 0;
    std::string current_atom = "";
    auto push_atom = [&]() {
        if (!current_atom.empty()) {
            if (current_atom == "or" || current_atom == "and" || current_atom == "not")
                tokens.push_back(Token{TokenType::Operator, current_atom});
            else
                tokens.push_back(Token{TokenType::Atom, current_atom});
            current_atom = "";
        }
    };
    bool inQuote = false;
    size_t typeBracketDepth = 0;
    bool isAtLineStart = true;
    size_t current_line_indent = 0;

    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];
        if (isAtLineStart) {
            if (c == ' ') {
                current_line_indent++;
                continue;
            } else if (c == '\t') {
                current_line_indent += 4;
                continue;
            } else if (c == '\n' || c == '\r') {
                current_line_indent = 0;
                continue;
            } else {
                isAtLineStart = false;
                int prev_indent = indent_stack.back();

                if (current_line_indent > prev_indent) {
                    indent_stack.push_back(current_line_indent);
                    tokens.push_back(Token{TokenType::Indent, "INDENT"});
                } else if (current_line_indent < prev_indent) {
                    while (indent_stack.back() > current_line_indent) {
                        indent_stack.pop_back();
                        tokens.push_back(Token{TokenType::Dedent, "DEDENT"});
                    }
                    if (indent_stack.back() != current_line_indent) {
                        throw std::invalid_argument("Indentation error!");
                    }
                }
            }
        }
        if (c == '\n') {
            push_atom();
            tokens.push_back(Token{TokenType::Newline, "\\n"});
            isAtLineStart = true;
            current_line_indent = 0;
            continue;
        }
        if (!inQuote && c == '<') {
            bool isType = false;
            if (!current_atom.empty()) {
                for (size_t j = i + 1; j < input.length(); ++j) {
                    if (input[j] == '>') {
                        isType = true;
                        break;
                    }
                    if (input[j] == '=' || input[j] == '<' || input[j] == '+' || input[j] == '-') {
                        break;
                    }
                }
            }

            if (isType) {
                ++typeBracketDepth;
                current_atom += c;
                continue;
            }
        }

        if (!inQuote && typeBracketDepth > 0 && c == '>') {
            --typeBracketDepth;
            current_atom += c;
            continue;
        }

        if (!inQuote && typeBracketDepth > 0) {
            if (!std::isspace(c))
                current_atom += c;
            continue;
        }

        if (c == '=' && (i + 1 >= input.length() || input[i + 1] != '=')) {
            push_atom();
            tokens.push_back(Token{TokenType::Operator, "="});
            continue;
        }
        if ((c == '=' || c == '!' || c == '<' || c == '>') && i + 1 < input.length() && input[i + 1] == '=') {
            push_atom();
            tokens.push_back(Token{TokenType::Operator, std::string(1, c) + "="});
            ++i;
            continue;
        }
        if (c == '>' || c == '<') {
            push_atom();
            tokens.push_back(Token{TokenType::Operator, std::string(1, c)});
            continue;
        }

        if (c == '"') {
            inQuote = !inQuote;
            current_atom += c;
            continue;
        }
        if (inQuote) {
            current_atom += c;
            continue;
        }
        if (std::isspace(c)) {
            push_atom();
            continue;
        }
        if (c == '*' && i + 1 < input.length() && input[i + 1] == '*') {
            push_atom();
            tokens.push_back(Token{TokenType::Operator, "**"});
            ++i;
            continue;
        }
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' || c == ',' || c == '.' || c == '[' ||
            c == ']' || c == ':') {
            push_atom();
            tokens.push_back(Token{TokenType::Operator, std::string(1, c)});
            continue;
        }

        current_atom += c;
    }
    push_atom();

    while (indent_stack.back() > 0) {
        indent_stack.pop_back();
        tokens.push_back(Token{TokenType::Dedent, "DEDENT"});
    }

    std::vector<ExprPtr> program_statements;
    while (peek().type != TokenType::Eof) {
        if (peek().type == TokenType::Newline) {
            consume();
            continue;
        }
        program_statements.push_back(std::make_unique<Expression>(parseStatement()));
    }

    return Expression{Program{std::move(program_statements)}, nullptr, nullptr};
}

std::string Lexer::astToString(const Expression &expr) {
    if (std::holds_alternative<Atom>(expr.value))
        return std::get<Atom>(expr.value).name;

    if (std::holds_alternative<Operator>(expr.value)) {
        std::string op = std::get<Operator>(expr.value).symbol;
        if (!expr.lhs && expr.rhs)
            return "(" + op + " " + astToString(*expr.rhs) + ")";
        if (expr.lhs && expr.rhs)
            return "(" + op + " " + astToString(*expr.lhs) + " " + astToString(*expr.rhs) + ")";
    } else if (std::holds_alternative<FunctionCall>(expr.value)) {
        const auto &func = std::get<FunctionCall>(expr.value);
        std::string result = "(call " + astToString(*func.target);
        for (const auto &arg: func.arguments)
            result += " " + astToString(*arg);
        result += ")";
        return result;
    } else if (std::holds_alternative<ArrayLiteral>(expr.value)) {
        const auto &arr = std::get<ArrayLiteral>(expr.value);
        std::string result = "[";
        for (size_t i = 0; i < arr.elements.size(); ++i) {
            result += astToString(*arr.elements[i]);
            if (i < arr.elements.size() - 1) result += " ";
        }
        result += "]";
        return result;
    } else if (std::holds_alternative<IndexAccess>(expr.value)) {
        const auto &idx = std::get<IndexAccess>(expr.value);
        return "(index " + astToString(*idx.array_expr) + " " + astToString(*idx.index_expr) + ")";
    } else if (std::holds_alternative<VarDeclaration>(expr.value)) {
        const auto &decl = std::get<VarDeclaration>(expr.value);
        std::string result = "(var " + decl.type_name + " " + decl.var_name;
        if (decl.value) {
            result += " " + astToString(*decl.value);
        }
        result += ")";
        return result;
    }
    return "UNKNOWN";
}

static std::string getIndent(int level) {
    return std::string(level * 4, ' ');
}

std::string Lexer::toCppString(const Expression &expr, int indentLevel,
                               const std::vector<Compiler::TypeBinder> *typeBinders) {
    if (std::holds_alternative<Atom>(expr.value)) {
        return std::get<Atom>(expr.value).name;
    } else if (std::holds_alternative<Block>(expr.value)) {
        const auto &block = std::get<Block>(expr.value);
        std::string result = "{\n";
        for (const auto &stmt: block.statements) {
            std::string stmt_str = toCppString(*stmt, indentLevel + 1, typeBinders);
            if (stmt_str.empty()) continue;

            result += getIndent(indentLevel + 1) + stmt_str;
            if (!std::holds_alternative<FunctionDeclaration>(stmt->value) &&
                !std::holds_alternative<IfStatement>(stmt->value) &&
                !std::holds_alternative<ForStatement>(stmt->value) &&
                !std::holds_alternative<UsingStatement>(stmt->value) &&
                stmt_str.back() != '\n') {
                result += ";\n";
            } else {
                result += "\n";
            }
        }
        result += getIndent(indentLevel) + "}";
        return result;
    } else if (std::holds_alternative<UsingStatement>(expr.value)) {
        const auto &use_stmt = std::get<UsingStatement>(expr.value);
        return "// using " + use_stmt.lib_name;
    } else if (std::holds_alternative<FunctionDeclaration>(expr.value)) {
        const auto &func = std::get<FunctionDeclaration>(expr.value);
        std::string result = "";

        std::string cpp_ret_type = func.return_type;
        if (func.name == "main") {
            cpp_ret_type = "int";
        } else if (typeBinders != nullptr && cpp_ret_type != "void" && cpp_ret_type != "int") {
            for (const auto &item: *typeBinders) {
                if (item.pandacName.empty()) continue;
                size_t pos = 0;
                bool isCompound = item.pandacName.find('<') != std::string::npos;
                while ((pos = cpp_ret_type.find(item.pandacName, pos)) != std::string::npos) {
                    bool leftOk = (pos == 0 || (!std::isalnum(cpp_ret_type[pos - 1]) && cpp_ret_type[pos - 1] != '_'));
                    bool rightOk = (pos + item.pandacName.size() >= cpp_ret_type.size() ||
                                    (!std::isalnum(cpp_ret_type[pos + item.pandacName.size()]) && cpp_ret_type[
                                         pos + item.pandacName.size()] != '_'));
                    if (isCompound || (leftOk && rightOk)) {
                        cpp_ret_type.replace(pos, item.pandacName.size(), item.cppName);
                        pos += item.cppName.size();
                    } else {
                        pos += item.pandacName.size();
                    }
                }
            }
        }

        result += getIndent(indentLevel) + cpp_ret_type + " " + func.name + "(";

        for (size_t i = 0; i < func.args.size(); ++i) {
            std::string arg_type = func.args[i].type;
            if (typeBinders != nullptr && arg_type != "auto") {
                for (const auto &item: *typeBinders) {
                    if (item.pandacName.empty()) continue;
                    size_t pos = 0;
                    bool isCompound = item.pandacName.find('<') != std::string::npos;
                    while ((pos = arg_type.find(item.pandacName, pos)) != std::string::npos) {
                        bool leftOk = (pos == 0 || (!std::isalnum(arg_type[pos - 1]) && arg_type[pos - 1] != '_'));
                        bool rightOk = (pos + item.pandacName.size() >= arg_type.size() ||
                                        (!std::isalnum(arg_type[pos + item.pandacName.size()]) && arg_type[
                                             pos + item.pandacName.size()] != '_'));
                        if (isCompound || (leftOk && rightOk)) {
                            arg_type.replace(pos, item.pandacName.size(), item.cppName);
                            pos += item.cppName.size();
                        } else
                            pos += item.pandacName.size();
                    }
                }
            }

            result += arg_type + " " + func.args[i].name;
            if (i < func.args.size() - 1) result += ", ";
        }

        result += ") \n";

        result += getIndent(indentLevel) + toCppString(*func.body, indentLevel, typeBinders);
        return result;
    } else if (std::holds_alternative<Program>(expr.value)) {
        const auto &prog = std::get<Program>(expr.value);
        std::string result = "";
        for (const auto &stmt: prog.statements) {
            std::string stmt_str = toCppString(*stmt, indentLevel, typeBinders);
            if (stmt_str.empty()) continue;

            result += stmt_str;
            if (!std::holds_alternative<FunctionDeclaration>(stmt->value) &&
                !std::holds_alternative<IfStatement>(stmt->value) &&
                !std::holds_alternative<UsingStatement>(stmt->value) &&
                !std::holds_alternative<ForStatement>(stmt->value) &&
                stmt_str.back() != '}' && stmt_str.back() != '\n') {
                result += ";\n";
            } else {
                result += "\n";
            }
        }
        return result;
    }
    else if (std::holds_alternative<WhileStatement>(expr.value)) {
        const auto& while_stmt = std::get<WhileStatement>(expr.value);
        std::string result = "while (" + toCppString(*while_stmt.condition, 0, typeBinders) + ") \n";
        result += getIndent(indentLevel) + toCppString(*while_stmt.body, indentLevel, typeBinders);
        return result;
    }
    else if (std::holds_alternative<IfStatement>(expr.value)) {
        const auto &if_stmt = std::get<IfStatement>(expr.value);

        std::string cond_str = toCppString(*if_stmt.condition, 0, typeBinders);
        std::string result = "";
        if (!cond_str.empty() && cond_str.front() == '(' && cond_str.back() == ')')
            result = "if " + cond_str + " \n";
        else
            result = "if (" + cond_str + ") \n";

        result += getIndent(indentLevel) + toCppString(*if_stmt.body, indentLevel, typeBinders);

        if (if_stmt.else_body) {
            if (std::holds_alternative<IfStatement>(if_stmt.else_body->value)) {
                result += " else " + toCppString(*if_stmt.else_body, indentLevel, typeBinders);
            } else {
                result += " else \n";
                result += getIndent(indentLevel) + toCppString(*if_stmt.else_body, indentLevel, typeBinders);
            }
        }

        return result;
    } else if (std::holds_alternative<ForStatement>(expr.value)) {
        const auto &for_stmt = std::get<ForStatement>(expr.value);
        std::string result = "for (auto " + for_stmt.iterator_name + " : " + toCppString(
                                 *for_stmt.collection, 0, typeBinders) + ") \n";
        result += getIndent(indentLevel) + toCppString(*for_stmt.body, indentLevel, typeBinders);
        return result;
    } else if (std::holds_alternative<Operator>(expr.value)) {
        std::string op = std::get<Operator>(expr.value).symbol;
        if (op == "=")
            return toCppString(*expr.lhs, indentLevel, typeBinders) + " = " + toCppString(
                       *expr.rhs, indentLevel, typeBinders);
        if (op == ".")
            return toCppString(*expr.lhs, indentLevel, typeBinders) + "." + toCppString(
                       *expr.rhs, indentLevel, typeBinders);
        else if (op == "and") op = "&&";
        else if (op == "or") op = "||";
        else if (op == "not") op = "!";
        if (!expr.lhs && expr.rhs)
            return "(" + op + toCppString(*expr.rhs, indentLevel, typeBinders) + ")";
        if (op == "**")
            return "std::pow(" + toCppString(*expr.lhs, indentLevel, typeBinders) + ", " + toCppString(
                       *expr.rhs, indentLevel, typeBinders) + ")";
        if (expr.lhs && expr.rhs)
            return "(" + toCppString(*expr.lhs, indentLevel, typeBinders) + " " + op + " " + toCppString(
                       *expr.rhs, indentLevel, typeBinders) + ")";
    } else if (std::holds_alternative<FunctionCall>(expr.value)) {
        const auto &func = std::get<FunctionCall>(expr.value);
        std::string result = toCppString(*func.target, indentLevel, typeBinders) + "(";

        for (size_t i = 0; i < func.arguments.size(); ++i) {
            result += toCppString(*func.arguments[i], indentLevel, typeBinders);
            if (i < func.arguments.size() - 1) {
                result += ", ";
            }
        }
        result += ")";
        return result;
    } else if (std::holds_alternative<ReturnStatement>(expr.value)) {
        const auto &ret = std::get<ReturnStatement>(expr.value);
        if (ret.value)
            return "return " + toCppString(*ret.value, indentLevel, typeBinders) + ";\n";
        else
            return "return;\n";
    } else if (std::holds_alternative<ArrayLiteral>(expr.value)) {
        const auto &arr = std::get<ArrayLiteral>(expr.value);
        std::string result = "{";
        for (size_t i = 0; i < arr.elements.size(); ++i) {
            result += toCppString(*arr.elements[i], indentLevel, typeBinders);
            if (i < arr.elements.size() - 1) {
                result += ", ";
            }
        }
        result += "}";
        return result;
    } else if (std::holds_alternative<VarDeclaration>(expr.value)) {
        const auto &decl = std::get<VarDeclaration>(expr.value);
        std::string result;

        std::string cpp_type = decl.type_name;
        if (typeBinders != nullptr) {
            for (const auto &item: *typeBinders) {
                if (item.pandacName.empty()) continue;
                size_t pos = 0;
                bool isCompound = item.pandacName.find('<') != std::string::npos;

                while ((pos = cpp_type.find(item.pandacName, pos)) != std::string::npos) {
                    bool leftOk = (pos == 0 || (!std::isalnum(cpp_type[pos - 1]) && cpp_type[pos - 1] != '_'));
                    bool rightOk = (pos + item.pandacName.size() >= cpp_type.size() ||
                                    (!std::isalnum(cpp_type[pos + item.pandacName.size()]) && cpp_type[
                                         pos + item.pandacName.size()] != '_'));

                    if (isCompound || (leftOk && rightOk)) {
                        cpp_type.replace(pos, item.pandacName.size(), item.cppName);
                        pos += item.cppName.size();
                    } else
                        pos += item.pandacName.size();
                }
            }
        }

        result = cpp_type + " " + decl.var_name;

        if (decl.value) {
            result += " = " + toCppString(*decl.value, indentLevel, typeBinders);
        }

        return result;
    } else if (std::holds_alternative<IndexAccess>(expr.value)) {
        const auto &idx = std::get<IndexAccess>(expr.value);
        return toCppString(*idx.array_expr, indentLevel, typeBinders) + "[" + toCppString(
                   *idx.index_expr, indentLevel, typeBinders) + "]";
    }
    return "";
}

Expression Lexer::parseStatement() {
    while (peek().type == TokenType::Newline)
        consume();
    Token first = peek();
    if (first.type == TokenType::Atom && (first.lexeme == "elif" || first.lexeme == "else")) {
        throw std::invalid_argument("Syntax error: '" + first.lexeme + "' without matching 'if'");
    }
    if (first.type == TokenType::Eof)
        throw std::invalid_argument("Unexpected EOF in parseStatement");
    if (first.type == TokenType::Atom && first.lexeme == "using") {
        consume();
        Token lib = consume();
        return Expression{UsingStatement{lib.lexeme}, nullptr, nullptr};
    } else if (first.type == TokenType::Atom && first.lexeme == "while") {
        consume();
        ExprPtr condition = std::make_unique<Expression>(parseExpression(0.0));

        if (peek().lexeme == ":") consume();
        while (peek().type == TokenType::Newline) consume();

        if (peek().type != TokenType::Indent)
            throw std::invalid_argument("Expected an indented block after 'while'");

        consume();
        ExprPtr body = std::make_unique<Expression>(parseBlock());

        if (peek().type == TokenType::Dedent) consume();
        while (peek().type == TokenType::Newline) consume();

        return Expression{WhileStatement{std::move(condition), std::move(body)}, nullptr, nullptr};
    } else if (first.type == TokenType::Atom && first.lexeme == "for") {
        consume();

        Token iter_tok = consume();
        std::string iterator_name = iter_tok.lexeme;

        Token in_tok = consume();
        if (in_tok.lexeme != "in") {
            throw std::invalid_argument("Expected 'in' after for iterator");
        }

        ExprPtr collection = std::make_unique<Expression>(parseExpression(0.0));

        if (peek().lexeme == ":") consume();
        while (peek().type == TokenType::Newline) consume();

        if (peek().type != TokenType::Indent)
            throw std::invalid_argument("Expected an indented block after 'for'");

        consume();
        ExprPtr body = std::make_unique<Expression>(parseBlock());
        if (peek().type == TokenType::Dedent) consume();
        while (peek().type == TokenType::Newline) consume();
        return Expression{ForStatement{iterator_name, std::move(collection), std::move(body)}, nullptr, nullptr};
    } else if (first.type == TokenType::Atom && first.lexeme == "def") {
        consume();

        Token first_token = consume();
        std::string return_type = "void";
        std::string func_name = first_token.lexeme;

        if (peek().lexeme != "(") {
            return_type = first_token.lexeme;
            Token name_token = consume();
            func_name = name_token.lexeme;
        }

        if (peek().lexeme != "(")
            throw std::invalid_argument("Expected '(' after function name");
        consume();

        std::vector<FunctionArg> args;
        if (peek().lexeme != ")") {
            while (true) {
                Token arg_first = consume();
                std::string arg_type = "auto";
                std::string arg_name = arg_first.lexeme;

                if (peek().type == TokenType::Atom) {
                    arg_type = arg_first.lexeme;
                    arg_name = consume().lexeme;
                }

                args.push_back({arg_type, arg_name});

                if (peek().lexeme == ",") {
                    consume();
                } else {
                    break;
                }
            }
        }

        if (peek().lexeme != ")")
            throw std::invalid_argument("Expected ')' after function arguments");

        consume();
        if (peek().lexeme == ":") consume();

        while (peek().type == TokenType::Newline) consume();

        if (peek().type != TokenType::Indent)
            throw std::invalid_argument("Expected an indented block after 'def'");
        consume();
        ExprPtr body = std::make_unique<Expression>(parseBlock());

        if (peek().type == TokenType::Dedent)
            consume();
        while (peek().type == TokenType::Newline) consume();
        return Expression{FunctionDeclaration{return_type, func_name, args, std::move(body)}, nullptr, nullptr};
    } else if (first.type == TokenType::Atom && first.lexeme == "if") {
        consume();
        ExprPtr condition = std::make_unique<Expression>(parseExpression(0.0));

        if (peek().lexeme == ":") consume();
        while (peek().type == TokenType::Newline) consume();

        if (peek().type != TokenType::Indent)
            throw std::invalid_argument("Expected an indented block after 'if'");

        consume();
        ExprPtr body = std::make_unique<Expression>(parseBlock());

        if (peek().type == TokenType::Dedent) consume();
        ExprPtr else_body = nullptr;
        ExprPtr *current_else_tail = &else_body;

        while (true) {
            while (peek().type == TokenType::Newline) consume();

            if (peek().type == TokenType::Atom && (peek().lexeme == "else" || peek().lexeme == "elif")) {
                Token else_tok = consume();

                if (else_tok.lexeme == "elif") {
                    ExprPtr elif_cond = std::make_unique<Expression>(parseExpression(0.0));

                    if (peek().lexeme == ":") consume();
                    while (peek().type == TokenType::Newline) consume();

                    if (peek().type != TokenType::Indent)
                        throw std::invalid_argument("Expected an indented block after 'elif'");

                    consume();
                    ExprPtr elif_body = std::make_unique<Expression>(parseBlock());

                    if (peek().type == TokenType::Dedent) consume();

                    ExprPtr new_elif = std::make_unique<Expression>(Expression{
                        IfStatement{std::move(elif_cond), std::move(elif_body), nullptr},
                        nullptr, nullptr
                    });

                    *current_else_tail = std::move(new_elif);
                    current_else_tail = &std::get<IfStatement>((*current_else_tail)->value).else_body;
                } else {
                    if (peek().lexeme == ":") consume();
                    while (peek().type == TokenType::Newline) consume();

                    if (peek().type != TokenType::Indent)
                        throw std::invalid_argument("Expected an indented block after 'else'");

                    consume();
                    *current_else_tail = std::make_unique<Expression>(parseBlock());

                    if (peek().type == TokenType::Dedent) consume();
                    while (peek().type == TokenType::Newline) consume();

                    break;
                }
            } else {
                std::cout << "BREAKING IF. NEXT TOKEN: " << peek().lexeme
                        << " TYPE: " << (int) peek().type << std::endl;
                break;
            }
        }
        return Expression{IfStatement{std::move(condition), std::move(body), std::move(else_body)}, nullptr, nullptr};
    } else if (first.type == TokenType::Atom && first.lexeme == "return") {
        consume();
        ExprPtr ret_val = nullptr;
        if (peek().type != TokenType::Eof && peek().type != TokenType::Newline)
            ret_val = std::make_unique<Expression>(parseExpression(0.0));
        if (peek().type == TokenType::Newline)
            consume();

        return Expression{ReturnStatement{std::move(ret_val)}, nullptr, nullptr};
    } else if (first.type == TokenType::Atom) {
        if (current_token_index + 1 < tokens.size()) {
            Token second = tokens[current_token_index + 1];
            if (second.type == TokenType::Atom) {
                consume();
                consume();
                std::string type_name = first.lexeme;
                std::string var_name = second.lexeme;
                ExprPtr init_value = nullptr;
                if (peek().lexeme == "=") {
                    consume();
                    init_value = std::make_unique<Expression>(parseExpression(0.0));
                }
                while (peek().type == TokenType::Newline)
                    consume();
                if (peek().type == TokenType::Newline) consume();
                return Expression{VarDeclaration{type_name, var_name, std::move(init_value)}, nullptr, nullptr};
            }
        }
    } else if (first.type == TokenType::Atom) {
        bool is_var_decl = false;
        bool is_assignment = false;

        if (current_token_index + 2 < tokens.size() &&
            tokens[current_token_index + 1].type == TokenType::Atom &&
            tokens[current_token_index + 2].lexeme == "=") {
            is_var_decl = true;
            } else if (current_token_index + 1 < tokens.size() &&
                       tokens[current_token_index + 1].lexeme == "=") {
                is_assignment = true;
                       }

        if (is_var_decl) {
            Token type_tok = consume();
            Token var_tok = consume();
            consume();
            ExprPtr init_value = std::make_unique<Expression>(parseExpression(0.0));
            while (peek().type == TokenType::Newline) consume();
            return Expression{VarDeclaration{type_tok.lexeme, var_tok.lexeme, std::move(init_value)}, nullptr, nullptr};
        } else if (is_assignment) {
            Token var_tok = consume();
            consume();
            ExprPtr val = std::make_unique<Expression>(parseExpression(0.0));
            while (peek().type == TokenType::Newline) consume();
            return Expression{
                Operator{"="},
                std::make_unique<Expression>(Atom{var_tok.lexeme}, nullptr, nullptr),
                std::move(val)
        };
        }
    }
    Expression expr = parseExpression(0.0);
    if (peek().type == TokenType::Newline)
        consume();
    return expr;
}


Expression Lexer::parseBlock() {
    std::vector<ExprPtr> statements;
    while (true) {
        Token next = peek();
        if (next.type == TokenType::Newline) {
            consume();
            continue;
        }
        if (next.type == TokenType::Eof ||
            next.type == TokenType::Dedent || next.type == TokenType::Indent ||
            next.lexeme == ")" || next.lexeme == "]" || next.lexeme == "," ||
            next.lexeme == ":")
            break;
        statements.push_back(std::make_unique<Expression>(parseStatement()));
    }

    return Expression{Block{std::move(statements)}, nullptr, nullptr};
}

Expression Lexer::parseExpression(float minBp) {
    Token token = consume();
    std::cout << "[PARSEEXPR START] minBp=" << minBp
            << " token.type=" << (int) token.type
            << " token.lexeme='" << token.lexeme << "'\n";
    if (token.type == TokenType::Newline || token.type == TokenType::Dedent || token.type == TokenType::Indent)
        return Expression{Atom{""}, nullptr, nullptr};
    Expression lhs;
    if (token.type == TokenType::Atom) {
        lhs = Expression{Atom{token.lexeme}, nullptr, nullptr};
    } else if (token.type == TokenType::Operator && token.lexeme == "(") {
        lhs = parseExpression(0.0);
        Token next = consume();
        if (next.lexeme != ")")
            throw std::invalid_argument("Expected ')' but got " + next.lexeme);
    } else if (token.type == TokenType::Operator && (token.lexeme == "-" || token.lexeme == "not")) {
        auto [lbp, rbp] = getBindingPower(token.lexeme);
        Expression rhs = parseExpression(token.lexeme == "-" ? 7.0f : 3.1f);
        lhs = Expression{Operator{token.lexeme}, nullptr, std::make_unique<Expression>(std::move(rhs))};
    } else if (token.type == TokenType::Operator && token.lexeme == "[") {
        std::vector<ExprPtr> elements;
        if (peek().lexeme != "]") {
            while (true) {
                elements.push_back(std::make_unique<Expression>(parseExpression(0.0)));
                if (peek().lexeme == ",") {
                    consume();
                } else {
                    break;
                }
            }
        }
        Token close_bracket = consume();
        if (close_bracket.lexeme != "]")
            throw std::invalid_argument("Expected ']' but got " + close_bracket.lexeme);

        lhs = Expression{ArrayLiteral{std::move(elements)}, nullptr, nullptr};
    } else {
        throw std::invalid_argument("Unexpected token: " + token.lexeme);
    }

    while (true) {
        Token next = peek();
        std::cout << "[PARSEEXPR LOOP] minBp=" << minBp
              << " next.type=" << (int)next.type
              << " next.lexeme='" << next.lexeme << "'\n";
        if (next.type == TokenType::Eof || next.type == TokenType::Newline ||
            next.type == TokenType::Dedent || next.type == TokenType::Indent ||
            next.lexeme == ")" || next.lexeme == "]" || next.lexeme == "," ||
            next.lexeme == ":")
            break;


        if (next.type != TokenType::Operator) {
            std::cout << "[PARSEEXPR] minBp=" << minBp
                    << " next.type=" << (int) next.type
                    << " next.lexeme='" << next.lexeme << "'\n";
            throw std::invalid_argument("Expected operator but got: " + next.lexeme);
        }
        auto [lbp, rbp] = getBindingPower(next.lexeme);
        if (lbp < minBp)
            break;
        if (next.lexeme == "(") {
            consume();

            ExprPtr target = std::make_unique<Expression>(std::move(lhs));

            bool isValidTarget = false;
            if (std::holds_alternative<Atom>(target->value))
                isValidTarget = true;
            else if (std::holds_alternative<Operator>(target->value)) {
                if (std::get<Operator>(target->value).symbol == ".")
                    isValidTarget = true;
            } else if (std::holds_alternative<IndexAccess>(target->value))
                isValidTarget = true;
            else if (std::holds_alternative<FunctionCall>(target->value))
                isValidTarget = true;

            if (!isValidTarget)
                throw std::invalid_argument("Left hand side is not a valid function target.");


            std::vector<ExprPtr> args;
            if (peek().lexeme != ")") {
                while (true) {
                    args.push_back(std::make_unique<Expression>(parseExpression(0.0)));
                    if (peek().lexeme == ",")
                        consume();
                    else
                        break;
                }
            }
            Token close_paren = consume();
            if (close_paren.lexeme != ")")
                throw std::invalid_argument("Expected ')' after function arguments but got " + close_paren.lexeme);

            lhs = Expression{FunctionCall{std::move(target), std::move(args)}, nullptr, nullptr};
        } else if (next.lexeme == "[") {
            consume();
            Expression indexExpr = parseExpression(0.0);
            Token close_bracket = consume();
            if (close_bracket.lexeme != "]")
                throw std::invalid_argument("Expected ']' after array index but got " + close_bracket.lexeme);
            lhs = Expression{
                IndexAccess{
                    std::make_unique<Expression>(std::move(lhs)),
                    std::make_unique<Expression>(std::move(indexExpr))
                },
                nullptr, nullptr
            };
        } else {
            consume();
            Expression right = parseExpression(rbp);

            Expression newLhs = Expression{
                Operator{next.lexeme},
                std::make_unique<Expression>(std::move(lhs)),
                std::make_unique<Expression>(std::move(right))
            };
            lhs = std::move(newLhs);
        }
    }

    return lhs;
}
