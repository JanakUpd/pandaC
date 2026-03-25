#include <gtest/gtest.h>
#include "lexer.h"

TEST(LexerTest, ParseMath) {
    Lexer lexer;
    Expression ast = lexer.fromString("a + b * c");

    std::string expected = "(program (+ a (* b c)))";
    EXPECT_EQ(Lexer::astToString(ast), expected);
}
TEST(LexerTest, ParseVarDeclaration) {
    Lexer lexer;
    Expression ast = lexer.fromString("int x = 10");

    std::string expected = "(program (var int x 10))";
    EXPECT_EQ(Lexer::astToString(ast), expected);
}
TEST(LexerTest, ParseFunctionCall) {
    Lexer lexer;
    Expression ast = lexer.fromString("print(\"Hello\", x)");

    std::string expected = "(program (call print \"Hello\" x))";
    EXPECT_EQ(Lexer::astToString(ast), expected);
}
TEST(LexerTest, ParseIfElse) {
    Lexer lexer;
    std::string code =
        "if x < 5:\n"
        "    print(x)\n"
        "else:\n"
        "    print(0)\n";

    Expression ast = lexer.fromString(code);
    std::string expected = "(program (if (< x 5) (block (call print x)) else (block (call print 0))))";
    EXPECT_EQ(Lexer::astToString(ast), expected);
}
TEST(LexerTest, CodeGeneration) {
    Lexer lexer;
    std::string code = "int x = 5";
    Expression ast = lexer.fromString(code);

    std::vector<Compiler::TypeBinder> binders;
    std::string cppCode = Lexer::toCppString(ast, 0, &binders);

    EXPECT_EQ(cppCode, "int x = 5;\n");
}
