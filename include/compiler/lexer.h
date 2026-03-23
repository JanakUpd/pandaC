#ifndef PANDAC_LEXER_H
#define PANDAC_LEXER_H
#include <memory>
#include <string>
#include <variant>
#include <vector>

enum class TokenType {
    Operator,
    Atom,
    Eof
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

struct Expression {
    std::variant<Atom, Operator> value;
    ExprPtr lhs = nullptr;
    ExprPtr rhs = nullptr;
};

class Lexer {
    std::vector<Token> tokens;
    std::pair<float, float> getBindingPower(const std::string& symbol);
    size_t current_token_index = 0;
    Token consume();
    Token peek();
    Expression parseExpression(float minBp);
    void replace(std::string& str, const std::string& from, const std::string& to);
public:
    static std::string toString(const Expression& expr);
    Expression fromString(std::string input);
};


#endif //PANDAC_LEXER_H