#include "lexer.h"
#include <string>
#include <vector>

std::pair<float, float> Lexer::getBindingPower(const std::string& symbol) {
    if (symbol == "or")  return {1.0, 1.1};
    if (symbol == "and") return {2.0, 2.1};
    if (symbol == "not") return {0.0, 3.1};
    if (symbol == "+")   return {4.0, 4.1};
    if (symbol == "-")   return {4.0, 4.1};
    if (symbol == "*")   return {5.0, 5.1};
    if (symbol == "/")   return {5.0, 5.1};
    if (symbol == "**")  return {6.1, 6.0};
    if (symbol == "(")   return {8.0, 0.0};
    if (symbol == ",")   return {0.0, 0.0};
    if (symbol == ")")   return {0.0, 0.0};
    if (symbol == "<")   return {3.2, 3.3};
    if (symbol == ">")   return {3.2, 3.3};
    if (symbol == "<=")   return {3.2, 3.3};
    if (symbol == ">=")   return {3.2, 3.3};
    if (symbol == "==")   return {3.2, 3.3};
    if (symbol == "!=")   return {3.2, 3.3};
    throw std::invalid_argument("invalid operator: " + symbol);
}
std::string getWhitespace(size_t length) {
    return std::string(length, ' ');
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
void Lexer::replace(std::string& str, const std::string& from, const std::string& to) {
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
    for (size_t i = 0; i < input.length(); ++i) {
        char& c = input[i];
        if ((c == '=' || c == '!' || c == '<' || c == '>') && i + 1 < input.length() && input[i+1] == '=') {
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
        if (c == '*' && i + 1 < input.length() && input[i+1] == '*') {
            push_atom();
            tokens.push_back(Token{TokenType::Operator, "**"});
            ++i;
            continue;
        }
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' || c == ',') {
            push_atom();
            tokens.push_back(Token{TokenType::Operator, std::string(1, c)});
            continue;
        }
        current_atom += c;
    }
    push_atom();

    Expression ast = parseExpression(0.0);

    Token last = peek();
    if (last.type != TokenType::Eof)
        throw std::invalid_argument("Unexpected token at end of expression: " + last.lexeme);

    return ast;
}

std::string Lexer::astToString(const Expression& expr) {
    if (std::holds_alternative<Atom>(expr.value))
        return std::get<Atom>(expr.value).name;
    if (std::holds_alternative<Operator>(expr.value)) {
        std::string op = std::get<Operator>(expr.value).symbol;
        if (!expr.lhs && expr.rhs)
            return "(" + op + " " + astToString(*expr.rhs) + ")";
        if (expr.lhs && expr.rhs)
            return "(" + op + " " + astToString(*expr.lhs) + " " + astToString(*expr.rhs) + ")";
    }
    else if (std::holds_alternative<FunctionCall>(expr.value)) {
        const auto& func = std::get<FunctionCall>(expr.value);
        std::string result = "(call " + func.name;
        for (const auto& arg : func.arguments)
            result += " " + astToString(*arg);
        result += ")";
        return result;
    }
    return "UNKNOWN";
}


std::string Lexer::toCppString(const Expression& expr) {
    if (std::holds_alternative<Atom>(expr.value)) {
        return std::get<Atom>(expr.value).name;
    }
    else if (std::holds_alternative<Operator>(expr.value)) {
        std::string op = std::get<Operator>(expr.value).symbol;
        if (op == "and") op = "&&";
        else if (op == "or") op = "||";
        else if (op == "not") op = "!";
        if (!expr.lhs && expr.rhs)
            return "(" + op + toCppString(*expr.rhs) + ")";
        if (op == "**")
            return "std::pow(" + toCppString(*expr.lhs) + ", " + toCppString(*expr.rhs) + ")";
        if (expr.lhs && expr.rhs)
            return "(" + toCppString(*expr.lhs) + " " + op + " " + toCppString(*expr.rhs) + ")";
    }
    else if (std::holds_alternative<FunctionCall>(expr.value)) {
        const auto& func = std::get<FunctionCall>(expr.value);
        std::string result = func.name + "(";

        for (size_t i = 0; i < func.arguments.size(); ++i) {
            result += toCppString(*func.arguments[i]);
            if (i < func.arguments.size() - 1) {
                result += ", ";
            }
        }
        result += ")";
        return result;
    }
    return "";
}

Expression Lexer::parseExpression(float minBp) {
    Token token = consume();
    Expression lhs;
    if (token.type == TokenType::Atom) {
        lhs = Expression{Atom{token.lexeme}, nullptr, nullptr};
    }
    else if (token.type == TokenType::Operator && token.lexeme == "(") {
        lhs = parseExpression(0.0);
        Token next = consume();
        if (next.lexeme != ")")
            throw std::invalid_argument("Expected ')' but got " + next.lexeme);
    }
    else if (token.type == TokenType::Operator && (token.lexeme == "-" || token.lexeme == "not")) {
        auto [lbp, rbp] = getBindingPower(token.lexeme);
        Expression rhs = parseExpression(token.lexeme == "-" ? 7.0f : 3.1f);
        lhs = Expression{Operator{token.lexeme}, nullptr, std::make_unique<Expression>(std::move(rhs))};
    }
    else {
        throw std::invalid_argument("Unexpected token: " + token.lexeme);
    }

    while (true) {
        Token next = peek();

        if (next.type == TokenType::Eof || next.lexeme == ")" || next.lexeme == ",")
            break;

        if (next.type != TokenType::Operator)
            throw std::invalid_argument("Expected operator but got: " + next.lexeme);
        auto [lbp, rbp] = getBindingPower(next.lexeme);
        if (lbp < minBp)
            break;
        if (next.lexeme == "(") {
            consume();
            std::string funcName = "";
            if (std::holds_alternative<Atom>(lhs.value)) {
                funcName = std::get<Atom>(lhs.value).name;
            }
            else {
                throw std::invalid_argument("Left hand side is not a valid function name.");
            }

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
            lhs = Expression{FunctionCall{funcName, std::move(args)}, nullptr, nullptr};
        }
        else {
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
