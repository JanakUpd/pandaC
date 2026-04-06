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
    if (symbol == "//") return {5.0, 5.1};
    if (symbol == "%") return {5.0, 5.1};
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
    if (symbol == "+=") return {0.1, 0.0};
    if (symbol == "-=") return {0.1, 0.0};
    if (symbol == "*=") return {0.1, 0.0};
    if (symbol == "/=") return {0.1, 0.0};
    if (symbol == "//=") return {0.1, 0.0};
    if (symbol == "%=") return {0.1, 0.0};
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

void Lexer::tokenize(const std::string &input) {
    tokens.clear();
    current_token_index = 0;
    scope_stack.clear();
    scope_stack.push_back({});
    std::string current_atom = "";
    auto push_atom = [&]() {
        if (!current_atom.empty()) {
            if (current_atom.front() == '"' && current_atom.back() == '"') {
                tokens.push_back(Token{TokenType::String, current_atom});
            } else if (current_atom == "or" || current_atom == "and" || current_atom == "not") {
                tokens.push_back(Token{TokenType::Operator, current_atom});
            } else {
                tokens.push_back(Token{TokenType::Atom, current_atom});
            }
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
        if (c == '"') {
            inQuote = !inQuote;
            current_atom += c;
            continue;
        }
        if (inQuote) {
            current_atom += c;
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
        if ((c == '=' || c == '!' || c == '<' || c == '>' || c == '+' || c == '-' || c == '*' || c == '/') && i + 1 <
            input.length() && input[i + 1] == '=') {
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
        if (c == '/' && i + 1 < input.length() && input[i + 1] == '/') {
            if (i + 2 < input.length() && input[i + 2] == '=') {
                push_atom();
                tokens.push_back(Token{TokenType::Operator, "//="});
                i += 2;
                continue;
            }
            push_atom();
            tokens.push_back(Token{TokenType::Operator, "//"});
            ++i;
            continue;
        }
        if (c == '%' && i + 1 < input.length() && input[i + 1] == '=') {
            push_atom();
            tokens.push_back(Token{TokenType::Operator, "%="});
            ++i;
            continue;
        }
        if (c == '.' && i + 1 < input.length() && std::isdigit(input[i + 1])) {
            current_atom += c;
            continue;
        }
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
            c == '(' || c == ')' ||
            c == '[' || c == ']' ||
            c == '{' || c == '}' ||
            c == ',' || c == '.' || c == ':') {
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
}

Expression Lexer::fromString(std::string input) {
    tokenize(input);

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

template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

std::string Lexer::astToString(const Expression &expr) {
    return std::visit(overloaded{
                          [](const Atom &atom) -> std::string {
                              return atom.name;
                          },
                          [](const StringLiteral &str) -> std::string {
                              return str.value;
                          },
                          [&](const DictLiteral &dict) -> std::string {
                              std::string result = "{";
                              for (size_t i = 0; i < dict.elements.size(); ++i) {
                                  result += astToString(*dict.elements[i].first) + ": " + astToString(
                                      *dict.elements[i].second);
                                  if (i < dict.elements.size() - 1) result += ", ";
                              }
                              result += "}";
                              return result;
                          },
                          [&](const Operator &op_val) -> std::string {
                              std::string op = op_val.symbol;
                              if (!expr.lhs && expr.rhs)
                                  return "(" + op + " " + astToString(*expr.rhs) + ")";
                              if (expr.lhs && expr.rhs)
                                  return "(" + op + " " + astToString(*expr.lhs) + " " + astToString(*expr.rhs) + ")";
                              return "(" + op + ")";
                          },
                          [&](const FunctionCall &func) -> std::string {
                              std::string result = "(call " + astToString(*func.target);
                              for (const auto &arg: func.arguments)
                                  result += " " + astToString(*arg);
                              result += ")";
                              return result;
                          },
                          [&](const ArrayLiteral &arr) -> std::string {
                              std::string result = "[";
                              for (size_t i = 0; i < arr.elements.size(); ++i) {
                                  result += astToString(*arr.elements[i]);
                                  if (i < arr.elements.size() - 1) result += " ";
                              }
                              result += "]";
                              return result;
                          },
                          [&](const IndexAccess &idx) -> std::string {
                              return "(index " + astToString(*idx.array_expr) + " " + astToString(*idx.index_expr) +
                                     ")";
                          },
                          [&](const VarDeclaration &decl) -> std::string {
                              std::string result = "(var " + decl.type_name + " " + decl.var_name;
                              if (decl.value) {
                                  result += " " + astToString(*decl.value);
                              }
                              result += ")";
                              return result;
                          },
                          [&](const Block &block) -> std::string {
                              std::string result = "(block";
                              for (const auto &stmt: block.statements)
                                  result += " " + astToString(*stmt);
                              result += ")";
                              return result;
                          },
                          [&](const Program &prog) -> std::string {
                              std::string result = "(program";
                              for (const auto &stmt: prog.statements)
                                  result += " " + astToString(*stmt);
                              result += ")";
                              return result;
                          },
                          [&](const WhileStatement &w_stmt) -> std::string {
                              return "(while " + astToString(*w_stmt.condition) + " " + astToString(*w_stmt.body) + ")";
                          },
                          [&](const ForStatement &f_stmt) -> std::string {
                              return "(for " + f_stmt.iterator_name + " " + astToString(*f_stmt.collection) + " " +
                                     astToString(*f_stmt.body) + ")";
                          },
                          [&](const IfStatement &if_stmt) -> std::string {
                              std::string result = "(if " + astToString(*if_stmt.condition) + " " + astToString(
                                                       *if_stmt.body);
                              if (if_stmt.else_body) result += " else " + astToString(*if_stmt.else_body);
                              result += ")";
                              return result;
                          },
                          [&](const FunctionDeclaration &func) -> std::string {
                              std::string result = "(def " + func.return_type + " " + func.name + " (args";
                              for (const auto &arg: func.args) result += " " + arg.type + ":" + arg.name;
                              result += ") " + astToString(*func.body) + ")";
                              return result;
                          },
                          [&](const ReturnStatement &ret) -> std::string {
                              if (ret.value) return "(return " + astToString(*ret.value) + ")";
                              return "(return)";
                          },
                          [](const UsingStatement &use_stmt) -> std::string {
                              return "(using " + use_stmt.lib_name + ")";
                          },
                          [](auto &&) -> std::string {
                              return "UNKNOWN";
                          }
                      }, expr.value);
}


static std::string getIndent(int level) {
    return std::string(level * 4, ' ');
}

std::string Lexer::toCppString(const Expression &expr, int indentLevel,
                               const std::vector<Compiler::TypeBinder> *typeBinders) {
    return std::visit(overloaded{
                          [&](const Atom &atom) -> std::string {
                              if (atom.name == "True") return "true";
                              if (atom.name == "False") return "false";
                              if (atom.name == "None") return "nullptr";
                              return atom.name;
                          },
                          [&](const StringLiteral &str) -> std::string {
                              return "std::string(" + str.value + ")";
                          },
                          [&](const Block &block) -> std::string {
                              std::string result = "{\n";
                              for (const auto &stmt: block.statements) {
                                  std::string stmt_str = toCppString(*stmt, indentLevel + 1, typeBinders);
                                  if (stmt_str.empty()) continue;

                                  result += getIndent(indentLevel + 1) + stmt_str;

                                  if (!std::holds_alternative<FunctionDeclaration>(stmt->value) &&
                                      !std::holds_alternative<IfStatement>(stmt->value) &&
                                      !std::holds_alternative<WhileStatement>(stmt->value) &&
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
                          },
                          [&](const DictLiteral &dict) -> std::string {
                              std::string result = "make_pandac_dict({";
                              for (size_t i = 0; i < dict.elements.size(); ++i) {
                                  // Добавлено PandaCKVPair перед внутренними скобками!
                                  result += "PandaCKVPair{" + toCppString(
                                              *dict.elements[i].first, indentLevel, typeBinders) + ", " +
                                          toCppString(*dict.elements[i].second, indentLevel, typeBinders) + "}";
                                  if (i < dict.elements.size() - 1) result += ", ";
                              }
                              result += "})";
                              return result;
                          },


                          [&](const UsingStatement &use_stmt) -> std::string {
                              return "// using " + use_stmt.lib_name;
                          },
                          [&](const FunctionDeclaration &func) -> std::string {
                              std::string cpp_ret_type = func.return_type;
                              if (func.name == "main") {
                                  cpp_ret_type = "int";
                              } else if (cpp_ret_type.empty()) cpp_ret_type = "auto";
                              else if (cpp_ret_type == "int") cpp_ret_type = "int64_t";
                              else if (cpp_ret_type == "str") cpp_ret_type = "std::string";
                              else if (cpp_ret_type == "float") cpp_ret_type = "double";
                              else if (typeBinders != nullptr && cpp_ret_type != "void" && cpp_ret_type != "int") {
                                  for (const auto &item: *typeBinders) {
                                      if (item.pandacName.empty()) continue;
                                      size_t pos = 0;
                                      bool isCompound = item.pandacName.find('<') != std::string::npos;
                                      while ((pos = cpp_ret_type.find(item.pandacName, pos)) != std::string::npos) {
                                          bool leftOk = (
                                              pos == 0 || (
                                                  !std::isalnum(cpp_ret_type[pos - 1]) && cpp_ret_type[pos - 1] !=
                                                  '_'));
                                          bool rightOk = (pos + item.pandacName.size() >= cpp_ret_type.size() ||
                                                          (!std::isalnum(cpp_ret_type[pos + item.pandacName.size()]) &&
                                                           cpp_ret_type[pos + item.pandacName.size()] != '_'));
                                          if (isCompound || (leftOk && rightOk)) {
                                              cpp_ret_type.replace(pos, item.pandacName.size(), item.cppName);
                                              pos += item.cppName.size();
                                          } else pos += item.pandacName.size();
                                      }
                                  }
                              }

                              std::string result = getIndent(indentLevel) + cpp_ret_type + " " + func.name + "(";
                              for (size_t i = 0; i < func.args.size(); ++i) {
                                  std::string arg_type = func.args[i].type;
                                  if (arg_type.empty()) arg_type = "auto";
                                  else if (arg_type == "int") arg_type = "int64_t";
                                  else if (arg_type == "str") arg_type = "std::string";
                                  else if (arg_type == "float") arg_type = "double";
                                  result += arg_type + " " + func.args[i].name;
                                  if (i < func.args.size() - 1) result += ", ";
                              }

                              result += ") \n" + getIndent(indentLevel);

                              std::string body_str = toCppString(*func.body, indentLevel, typeBinders);

                              if (!body_str.empty() && body_str.back() == '}') {
                                  std::string default_ret = (func.name == "main") ? "return 0;" : "return;";
                                  body_str.pop_back();
                                  body_str += getIndent(indentLevel + 1) + default_ret + "\n" + getIndent(indentLevel) +
                                          "}";
                              }

                              result += body_str;
                              return result;
                          },
                          [&](const Program &prog) -> std::string {
                              std::string result = "";
                              for (const auto &stmt: prog.statements) {
                                  std::string stmt_str = toCppString(*stmt, indentLevel, typeBinders);
                                  if (stmt_str.empty()) continue;

                                  result += stmt_str;
                                  if (!std::holds_alternative<FunctionDeclaration>(stmt->value) &&
                                      !std::holds_alternative<IfStatement>(stmt->value) &&
                                      !std::holds_alternative<WhileStatement>(stmt->value) &&
                                      !std::holds_alternative<UsingStatement>(stmt->value) &&
                                      !std::holds_alternative<ForStatement>(stmt->value) &&
                                      stmt_str.back() != '}' && stmt_str.back() != '\n') {
                                      result += ";\n";
                                  } else {
                                      result += "\n";
                                  }
                              }
                              return result;
                          },
                          [&](const WhileStatement &while_stmt) -> std::string {
                              return "while (pandac_bool(" + toCppString(*while_stmt.condition, 0, typeBinders) +
                                     ")) \n" +
                                     getIndent(indentLevel) + toCppString(*while_stmt.body, indentLevel, typeBinders);
                          },

                          [&](const IfStatement &if_stmt) -> std::string {
                              std::string cond_str = toCppString(*if_stmt.condition, 0, typeBinders);
                              std::string result = "if (pandac_bool(" + cond_str + ")) \n";

                              result += getIndent(indentLevel) + toCppString(*if_stmt.body, indentLevel, typeBinders);


                              if (if_stmt.else_body) {
                                  if (std::holds_alternative<IfStatement>(if_stmt.else_body->value)) {
                                      result += " else " + toCppString(*if_stmt.else_body, indentLevel, typeBinders);
                                  } else {
                                      result += " else \n" + getIndent(indentLevel) + toCppString(
                                          *if_stmt.else_body, indentLevel, typeBinders);
                                  }
                              }
                              return result;
                          },
                          [&](const ForStatement &for_stmt) -> std::string {
                              return "for (auto " + for_stmt.iterator_name + " : " + toCppString(
                                         *for_stmt.collection, 0, typeBinders) + ") \n" +
                                     getIndent(indentLevel) + toCppString(*for_stmt.body, indentLevel, typeBinders);
                          },
                          [&](const Operator &op_val) -> std::string {
                              std::string op = op_val.symbol;

                              if (op == ".")
                                  return toCppString(*expr.lhs, indentLevel, typeBinders) + "." + toCppString(
                                             *expr.rhs, indentLevel, typeBinders);

                              if (!expr.lhs && expr.rhs) {
                                  std::string rhs_str = toCppString(*expr.rhs, indentLevel, typeBinders);
                                  if (op == "not") return "pandac_negate(" + rhs_str + ")";
                                  return "(" + op + " " + rhs_str + ")";
                              }

                              if (expr.lhs && expr.rhs) {
                                  std::string lhs_str = toCppString(*expr.lhs, indentLevel, typeBinders);
                                  std::string rhs_str = toCppString(*expr.rhs, indentLevel, typeBinders);
                                  if (op == "=") return "pandac_assign(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "+=") return "pandac_assign_add(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "-=") return "pandac_assign_sub(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "*=") return "pandac_assign_mul(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "/=") return "pandac_assign_div(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "//=") return "pandac_assign_int_div(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "%=") return "pandac_assign_mod(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "**") return "pandac_pow(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "and") return "pandac_and(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "or") return "pandac_or(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "not") return "pandac_negate(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "+") return "pandac_add(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "-") return "pandac_sub(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "*") return "pandac_mul(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "/") return "pandac_div(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "//") return "pandac_int64_div(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "%") return "pandac_mod(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "==") return "pandac_eq(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "!=") return "pandac_neq(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "<") return "pandac_less(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == "<=") return "pandac_less_eq(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == ">") return "pandac_greater(" + lhs_str + ", " + rhs_str + ")";
                                  if (op == ">=") return "pandac_greater_eq(" + lhs_str + ", " + rhs_str + ")";
                                  return "(" + lhs_str + " " + op + " " + rhs_str + ")";
                              }

                              return "";
                          },

                          [&](const FunctionCall &func) -> std::string {
                              if (std::holds_alternative<Operator>(func.target->value)) {
                                  const auto &op = std::get<Operator>(func.target->value);
                                  if (op.symbol == ".") {
                                      std::string obj_str = toCppString(*func.target->lhs, indentLevel, typeBinders);
                                      std::string method_name =
                                              toCppString(*func.target->rhs, indentLevel, typeBinders);

                                      std::string args_str = "{";
                                      for (size_t i = 0; i < func.arguments.size(); ++i) {
                                          args_str += toCppString(*func.arguments[i], indentLevel, typeBinders);
                                          if (i < func.arguments.size() - 1) args_str += ", ";
                                      }
                                      args_str += "}";

                                      return obj_str + ".callMethod(\"" + method_name + "\", " + args_str + ")";
                                  }
                              }
                              std::string target_name = toCppString(*func.target, indentLevel, typeBinders);

                              if (target_name == "str") target_name = "pandac_str";
                              else if (target_name == "int") target_name = "pandac_int";
                              else if (target_name == "float") target_name = "pandac_float";
                              else if (target_name == "bool") target_name = "pandac_bool";
                              else if (target_name == "len") target_name = "pandac_len";
                              else if (target_name == "print") target_name = "pandac_print";

                              std::string result = target_name + "(";
                              for (size_t i = 0; i < func.arguments.size(); ++i) {
                                  result += toCppString(*func.arguments[i], indentLevel, typeBinders);
                                  if (i < func.arguments.size() - 1) result += ", ";
                              }
                              result += ")";
                              return result;
                          },
                          [&](const ReturnStatement &ret) -> std::string {
                              if (ret.value)
                                  return
                                          "return " + toCppString(*ret.value, indentLevel, typeBinders) + ";\n";
                              return "return;\n";
                          },
                          [&](const ArrayLiteral &arr) -> std::string {
                              std::string result = "make_pandac_list({";
                              for (size_t i = 0; i < arr.elements.size(); ++i) {
                                  result += toCppString(*arr.elements[i], indentLevel, typeBinders);
                                  if (i < arr.elements.size() - 1) result += ", ";
                              }
                              result += "})";
                              return result;
                          },
                          [&](const VarDeclaration &decl) -> std::string {
                              std::string cpp_type = decl.type_name;

                              if (cpp_type == "int" || cpp_type == "int64") cpp_type = "int64_t";
                              else if (cpp_type == "float" || cpp_type == "double") cpp_type = "double";
                              else if (cpp_type == "str" || cpp_type == "string") cpp_type = "std::string";
                              else if (cpp_type == "bool") cpp_type = "bool";
                              else if (cpp_type == "list") cpp_type = "PandaCList";
                              else if (cpp_type == "dict") cpp_type = "PandaCDict";

                              else if (cpp_type.empty() || cpp_type == "auto") cpp_type = "auto";

                              if (typeBinders != nullptr) {
                                  for (const auto &item: *typeBinders) {
                                      if (item.pandacName.empty()) continue;
                                      size_t pos = 0;
                                      bool isCompound = item.pandacName.find(" ") != std::string::npos;
                                      while ((pos = cpp_type.find(item.pandacName, pos)) != std::string::npos) {
                                          bool leftOk =
                                                  (pos == 0) || !std::isalnum(cpp_type[pos - 1]) && cpp_type[pos - 1] !=
                                                  '_';
                                          bool rightOk =
                                                  (pos + item.pandacName.size() == cpp_type.size()) || !std::isalnum(
                                                      cpp_type[pos + item.pandacName.size()]) && cpp_type[
                                                      pos + item.pandacName.size()] != '_';
                                          if (isCompound || (leftOk && rightOk)) {
                                              cpp_type.replace(pos, item.pandacName.size(), item.cppName);
                                              pos += item.cppName.size();
                                          } else {
                                              pos += item.pandacName.size();
                                          }
                                      }
                                  }
                              }

                              std::string result = cpp_type + " " + decl.var_name;
                              if (decl.value) {
                                  result += " = " + toCppString(*decl.value, indentLevel, typeBinders);
                              }
                              return result;
                          },
                          [&](const IndexAccess &idx) -> std::string {
                              return toCppString(*idx.array_expr, indentLevel, typeBinders) +
                                     "[" + toCppString(*idx.index_expr, indentLevel, typeBinders) + "]";
                          },
                          [&](auto &&) -> std::string {
                              return "/* UNHANDLED AST NODE */";
                          }
                      }, expr.value);
}

Expression Lexer::parseUsingStatement() {
    consume();
    Token lib = consume();
    return Expression{UsingStatement{lib.lexeme}, nullptr, nullptr};
}

Expression Lexer::parseWhileStatement() {
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
}

Expression Lexer::parseForStatement() {
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
}

Expression Lexer::parseFunctionDeclaration() {
    consume();

    Token first_token = consume();
    std::string return_type = "auto";
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
    enterScope();
    for (const auto &arg: args) {
        declareVar(arg.name);
    }
    consume();
    ExprPtr body = std::make_unique<Expression>(parseBlock());
    exitScope();
    if (peek().type == TokenType::Dedent)
        consume();
    while (peek().type == TokenType::Newline) consume();
    return Expression{FunctionDeclaration{return_type, func_name, args, std::move(body)}, nullptr, nullptr};
}

Expression Lexer::parseIfStatement() {
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
            break;
        }
    }
    return Expression{IfStatement{std::move(condition), std::move(body), std::move(else_body)}, nullptr, nullptr};
}

Expression Lexer::parseReturnStatement() {
    consume();
    ExprPtr ret_val = nullptr;
    if (peek().type != TokenType::Eof && peek().type != TokenType::Newline)
        ret_val = std::make_unique<Expression>(parseExpression(0.0));
    if (peek().type == TokenType::Newline)
        consume();

    return Expression{ReturnStatement{std::move(ret_val)}, nullptr, nullptr};
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
        return parseUsingStatement();
    } else if (first.type == TokenType::Atom && first.lexeme == "while") {
        return parseWhileStatement();
    } else if (first.type == TokenType::Atom && first.lexeme == "for") {
        return parseForStatement();
    } else if (first.type == TokenType::Atom && first.lexeme == "def") {
        return parseFunctionDeclaration();
    } else if (first.type == TokenType::Atom && first.lexeme == "if") {
        return parseIfStatement();
    } else if (first.type == TokenType::Atom && first.lexeme == "return") {
        return parseReturnStatement();
    } else if (first.type == TokenType::Atom || first.type == TokenType::Operator) {
        if (first.type == TokenType::Operator) {
            bool isValidStart = (first.lexeme == "(" || first.lexeme == "{" ||
                                 first.lexeme == "[" || first.lexeme == "-" ||
                                 first.lexeme == "not" || first.lexeme == "+");

            if (!isValidStart) {
                throw std::invalid_argument(
                    "Syntax error: Statement cannot start with operator '" + first.lexeme + "' (Variables missing?)");
            }
        }

        size_t saved_index = current_token_index;

        bool is_typed_decl = false;
        if (current_token_index + 2 < tokens.size()) {
            if (tokens[current_token_index + 1].type == TokenType::Atom &&
                tokens[current_token_index + 2].lexeme == "=") {
                is_typed_decl = true;
            }
        }

        if (is_typed_decl) {
            Token type_tok = consume();
            Token var_tok = consume();
            consume();
            ExprPtr init_value = std::make_unique<Expression>(parseExpression(0.0));
            while (peek().type == TokenType::Newline) consume();
            return Expression{VarDeclaration{type_tok.lexeme, var_tok.lexeme, std::move(init_value)}, nullptr, nullptr};
        }

        current_token_index = saved_index;
        Expression left_expr = parseExpression(0.0);

        if (peek().type == TokenType::Newline || peek().type == TokenType::Eof) {
            while (peek().type == TokenType::Newline) consume();
            return left_expr;
        }

        std::string next_lex = peek().lexeme;
        if (next_lex == "=" || next_lex == "+=" || next_lex == "-=" ||
            next_lex == "*=" || next_lex == "/=") {
            std::string assign_op = consume().lexeme;

            ExprPtr right_expr = std::make_unique<Expression>(parseExpression(0.0));
            while (peek().type == TokenType::Newline) consume();

            if (std::holds_alternative<Atom>(left_expr.value) && assign_op == "=") {
                std::string var_name = std::get<Atom>(left_expr.value).name;
                if (!isDeclared(var_name)) {
                    declareVar(var_name);
                    return Expression{VarDeclaration{"auto", var_name, std::move(right_expr)}, nullptr, nullptr};
                }
            }

            return Expression{
                Operator{assign_op}, std::make_unique<Expression>(std::move(left_expr)), std::move(right_expr)
            };
        }

        throw std::invalid_argument("Unexpected token after expression: " + peek().lexeme);
    } else {
        throw std::invalid_argument("Syntax error: Unrecognized statement starting with " + first.lexeme);
    }
}

bool Lexer::isDeclared(const std::string &varName) {
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it)
        if (it->find(varName) != it->end())
            return true;
    return false;
}

void Lexer::declareVar(const std::string &varName) {
    if (!scope_stack.empty())
        scope_stack.back().insert(varName);
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
    if (token.type == TokenType::Newline || token.type == TokenType::Dedent || token.type == TokenType::Indent)
        return Expression{Atom{""}, nullptr, nullptr};
    Expression lhs;
    if (token.type == TokenType::Atom) {
        lhs = Expression{Atom{token.lexeme}, nullptr, nullptr};
    } else if (token.type == TokenType::String) {
        lhs = Expression{StringLiteral{token.lexeme}, nullptr, nullptr};
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
    } else if (token.type == TokenType::Operator && token.lexeme == "{") {
        std::vector<std::pair<ExprPtr, ExprPtr> > elements;
        while (peek().type == TokenType::Newline || peek().type == TokenType::Indent) consume();
        if (peek().lexeme != "}" && peek().type != TokenType::Dedent) {
            while (true) {
                if (peek().lexeme == "}" || peek().type == TokenType::Dedent) break;

                ExprPtr key = std::make_unique<Expression>(parseExpression(0.0f));

                while (peek().type == TokenType::Newline) consume();
                Token colon = consume();
                if (colon.lexeme != ":") throw std::invalid_argument("Expected ':' in dictionary");

                while (peek().type == TokenType::Newline) consume();
                ExprPtr val = std::make_unique<Expression>(parseExpression(0.0f));

                elements.push_back({std::move(key), std::move(val)});

                while (peek().type == TokenType::Newline) consume();
                if (peek().lexeme == ",") {
                    consume();
                    while (peek().type == TokenType::Newline || peek().type == TokenType::Indent) consume();
                } else {
                    break;
                }
            }
        }

        while (peek().type == TokenType::Newline || peek().type == TokenType::Dedent) consume();

        Token closebrace = consume();
        if (closebrace.lexeme != "}") throw std::invalid_argument("Expected '}' after dict");
        lhs = Expression{DictLiteral{std::move(elements)}, nullptr, nullptr};
    } else {
        throw std::invalid_argument("Unexpected token: " + token.lexeme);
    }

    while (true) {
        Token next = peek();
        if (next.type == TokenType::Eof || next.type == TokenType::Newline ||
            next.type == TokenType::Dedent || next.type == TokenType::Indent ||
            next.lexeme == ")" || next.lexeme == "]" || next.lexeme == "}" ||
            next.lexeme == "," || next.lexeme == ":" ||
            next.lexeme == "=" || next.lexeme == "+=" || next.lexeme == "-=" ||
            next.lexeme == "*=" || next.lexeme == "/=")
            break;


        if (next.type != TokenType::Operator) {
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
