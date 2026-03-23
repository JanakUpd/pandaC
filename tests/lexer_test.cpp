#include <iostream>
#include <cassert>
#include "lexer.h"

void run_tests() {
    Lexer lexer;
    int tests_passed = 0;

    auto run_test = [&](const std::string& name, const std::string& input, const std::string& expected) {
        try {
            Expression ast = lexer.fromString(input);
            std::string result = Lexer::toString(ast);
            if (result == expected) {
                std::cout << "[PASS] " << name << ": " << input << " -> " << result << std::endl;
                tests_passed++;
            } else {
                std::cerr << "[FAIL] " << name << ":\n"
                          << "  Input:    " << input << "\n"
                          << "  Expected: " << expected << "\n"
                          << "  Actual:   " << result << std::endl;
                assert(false);
            }
        } catch (const std::exception& e) {
            std::cerr << "[FAIL] " << name << " threw exception: " << e.what() << std::endl;
            assert(false);
        }
    };
    run_test("Simple Add", "a + b", "(+ a b)");
    run_test("Simple Mul", "a * b", "(* a b)");
    run_test("Precedence", "a + b * c", "(+ a (* b c))");
    run_test("Parens", "(a + b) * c", "(* (+ a b) c)");
    run_test("Unary Minus 1", "-a * b", "(* (- a) b)");
    run_test("Unary Minus 2", "a * -b", "(* a (- b))");
    run_test("Unary Not 1", "not a and b", "(and (not a) b)");
    run_test("Unary Not 2", "a or not b", "(or a (not b))");
    run_test("Multiple Unary", "not not -a", "(not (not (- a)))");
    run_test("Unary before Parens", "-(a + b)", "(- (+ a b))");
    run_test("Left Assoc Sub", "a - b - c", "(- (- a b) c)");
    run_test("Left Assoc Div", "a / b / c", "(/ (/ a b) c)");
    run_test("Right Assoc Pow", "a ** b ** c", "(** a (** b c))");
    run_test("Logical Precedence", "a or b and c", "(or a (and b c))");
    run_test("Complex Logic", "a and b or c and not d", "(or (and a b) (and c (not d)))");
    run_test("Deep Parens", "(((a)))", "a");
    run_test("Complex Math",
             "a + b * c ** d / e - f",
             "(- (+ a (/ (* b (** c d)) e)) f)");
    run_test("Crazy Mix",
             "-a ** 2 * (b + c) / not d and e",
             "(and (/ (* (** (- a) 2) (+ b c)) (not d)) e)");
    auto run_fail_test = [&](const std::string& name, const std::string& input) {
        try {
            lexer.fromString(input);
            std::cerr << "[FAIL] " << name << " should have thrown an exception but didn't!" << std::endl;
            assert(false);
        } catch (const std::invalid_argument& e) {
            std::cout << "[PASS] " << name << " correctly threw: " << e.what() << std::endl;
            tests_passed++;
        }
    };

    run_fail_test("Unmatched Parens 1", "(a + b");
    run_fail_test("Unmatched Parens 2", "a + b)");
    run_fail_test("Empty Parens", "()");
    run_fail_test("Missing Operand", "a + * b");
    run_fail_test("Trailing Operator", "a + b +");
}


int main() {
    try {
        run_tests();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
