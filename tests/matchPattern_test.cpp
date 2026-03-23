#include <gtest/gtest.h>
#include "codeconvertion.h"

class MatchPatternTest : public ::testing::Test {
protected:
    std::vector<std::string> params;

    void SetUp() override {
        params.clear();
    }
};

TEST_F(MatchPatternTest, BasicExactMatch) {
    EXPECT_TRUE(CodeConvertion::matchPattern("exit()", "exit()", params));
    EXPECT_TRUE(params.empty());
}

TEST_F(MatchPatternTest, SingleParameterCapture) {
    std::string line = "print(\"Hello World\")";
    std::string pattern = "print([0])";

    EXPECT_TRUE(CodeConvertion::matchPattern(line, pattern, params));
    ASSERT_EQ(params.size(), 1);
    EXPECT_EQ(params[0], "\"Hello World\"");
}

TEST_F(MatchPatternTest, MultipleParameters) {
    std::string line = "func(10, 20)";
    std::string pattern = "func([0], [1])";

    EXPECT_TRUE(CodeConvertion::matchPattern(line, pattern, params));
    ASSERT_EQ(params.size(), 2);
    EXPECT_EQ(params[0], "10");
    EXPECT_EQ(params[1], "20");
}

TEST_F(MatchPatternTest, NestedParentheses) {
    std::string line = "calc((a + b) * c)";
    std::string pattern = "calc([0])";

    EXPECT_TRUE(CodeConvertion::matchPattern(line, pattern, params));
    ASSERT_EQ(params.size(), 1);
    EXPECT_EQ(params[0], "(a + b) * c");
}

TEST_F(MatchPatternTest, DelimiterInsideQuotes) {
    std::string line = "log(\"Error, unexpected\", true)";
    std::string pattern = "log([0], [1])";

    EXPECT_TRUE(CodeConvertion::matchPattern(line, pattern, params));
    ASSERT_EQ(params.size(), 2);
    EXPECT_EQ(params[0], "\"Error, unexpected\"");
    EXPECT_EQ(params[1], "true");
}

TEST_F(MatchPatternTest, ReorderedIndices) {
    std::string line = "swap(first, second)";
    std::string pattern = "swap([1], [0])";

    EXPECT_TRUE(CodeConvertion::matchPattern(line, pattern, params));
    ASSERT_EQ(params.size(), 2);
    EXPECT_EQ(params[0], "second");
    EXPECT_EQ(params[1], "first");
}

TEST_F(MatchPatternTest, MismatchPattern) {
    EXPECT_FALSE(CodeConvertion::matchPattern("print(x)", "scan([0])", params));
}
