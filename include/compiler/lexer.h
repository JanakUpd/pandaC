#ifndef PANDAC_LEXER_H
#define PANDAC_LEXER_H
#include <memory>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

#include "compiler.h"

enum class TokenType {
    Operator,
    Atom,
    String,
    Eof,
    Newline,
    Indent,
    Dedent
};

struct Token {
    TokenType type;
    std::string lexeme;
};
struct Atom {
    std::string name;
};
struct Operator {
    std::string symbol;
};
struct Expression;
using ExprPtr = std::unique_ptr<Expression>;

struct VarDeclaration {
    std::string type_name;
    std::string var_name;
    ExprPtr value;
};

struct ArrayLiteral {
    std::vector<ExprPtr> elements;
};

struct IndexAccess {
    ExprPtr array_expr;
    ExprPtr index_expr;
};

struct FunctionCall {
    ExprPtr target;
    std::vector<ExprPtr> arguments;
};

struct ReturnStatement {
    ExprPtr value;
};
struct Block {
    std::vector<ExprPtr> statements;
};

struct IfStatement {
    ExprPtr condition;
    ExprPtr body;
    ExprPtr else_body;
};
struct UsingStatement {
    std::string lib_name;
};
struct Program {
    std::vector<ExprPtr> statements;
};
struct FunctionArg {
    std::string type;
    std::string name;
};

struct FunctionDeclaration {
    std::string return_type;
    std::string name;
    std::vector<FunctionArg> args;
    ExprPtr body;
};

struct ForStatement {
    std::string iterator_name;
    ExprPtr collection;
    ExprPtr body;
};

struct WhileStatement {
    ExprPtr condition;
    ExprPtr body;
};
struct StringLiteral {
    std::string value;
};
struct DictLiteral {
    std::vector<std::pair<ExprPtr, ExprPtr>> elements;
};

struct Expression {
    std::variant<
        Atom,
        Operator,
        FunctionCall,
        ArrayLiteral,
        IndexAccess,
        VarDeclaration,
        ReturnStatement,
        IfStatement,
        Block,
        UsingStatement,
        FunctionDeclaration,
        Program,
        ForStatement,
        WhileStatement,
        StringLiteral,
        DictLiteral
    > value;
    ExprPtr lhs = nullptr;
    ExprPtr rhs = nullptr;
};

class Lexer {
    std::pair<float, float> getBindingPower(const std::string& symbol);
    size_t current_token_index = 0;
    std::vector<int> indent_stack = {0};
    Token consume();
    Token peek();
    Expression parseStatement();
    Expression parseBlock();
    Expression parseExpression(float minBp);
    void replace(std::string& str, const std::string& from, const std::string& to);
    std::vector<std::unordered_set<std::string>> scope_stack;
    void enterScope() { scope_stack.push_back({}); }
    void exitScope() { if (!scope_stack.empty()) scope_stack.pop_back(); }
    bool isDeclared(const std::string& varName);
    void declareVar(const std::string& varName);
public:
    std::vector<Token> tokens;
    static std::string astToString(const Expression& expr);
    static std::string toCppString(const Expression& expr, int indentLevel = 0, const std::vector<Compiler::TypeBinder>* typeBinders = nullptr);
    Expression fromString(std::string input);
private:
    void tokenize(const std::string& input);
    Expression parseUsingStatement();
    Expression parseWhileStatement();
    Expression parseForStatement();
    Expression parseIfStatement();
    Expression parseFunctionDeclaration();
    Expression parseReturnStatement();
};


#endif //PANDAC_LEXER_H