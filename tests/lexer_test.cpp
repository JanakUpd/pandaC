#include <iostream>
#include <cassert>
#include "lexer.h"
void run_test(const std::string& name, const std::string& input, const std::string& expected) {
    std::cout << "[ RUN      ] " << name << "\n";
    Lexer lexer;
    Expression ast = lexer.fromString(input);
    std::string result = Lexer::astToString(ast);
    if (result == expected) {
        std::cout << "[PASS] " << name << ": " << input << " -> " << result << std::endl;
    } else {
        std::cerr << "[FAIL] " << name << ":\n"
                  << "  Input:    " << input << "\n"
                  << "  Expected: " << expected << "\n"
                  << "  Actual:   " << result << std::endl;
        assert(false);
    }
    if (result == expected) {
        std::cout << "[       OK ] " << name << "\n";
    } else {
        std::cout << "[  FAILED  ] " << name << "\n";
    }
}

void run_tests() {
    Lexer lexer;
    int tests_passed = 0;
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
    // auto run_fail_test = [&](const std::string& name, const std::string& input) {
    //     try {
    //         lexer.fromString(input);
    //         std::cerr << "[FAIL] " << name << " should have thrown an exception but didn't!" << std::endl;
    //         assert(false);
    //     } catch (const std::invalid_argument& e) {
    //         std::cout << "[PASS] " << name << " correctly threw: " << e.what() << std::endl;
    //         tests_passed++;
    //     }
    // };
    //
    // run_fail_test("Unmatched Parens 1", "(a + b");
    // run_fail_test("Unmatched Parens 2", "a + b)");
    // run_fail_test("Empty Parens", "()");
    // run_fail_test("Missing Operand", "a + * b");
    // run_fail_test("Trailing Operator", "a + b +");

    run_test("Func No Args",
             "print()",
             "(call print)");
    run_test("Func One Arg",
             "sin(x)",
             "(call sin x)");
    run_test("Func Multiple Args",
             "min(a, b, c)",
             "(call min a b c)");
    run_test("Func Math Arg",
             "sin(a + b * c)",
             "(call sin (+ a (* b c)))");
    run_test("Math with Func",
             "a + sin(b) * c",
             "(+ a (* (call sin b) c))");
    run_test("Nested Funcs",
             "print(sin(max(a, b)))",
             "(call print (call sin (call max a b)))");
    run_test("Combo Func and Math",
         "-math.sqrt(a ** 2 + b ** 2)",
         "(- (call (. math sqrt) (+ (** a 2) (** b 2))))");
    // run_fail_test("Func Missing Paren", "sin(a");
    // run_fail_test("Func Extra Comma", "sin(a,)");
    // run_fail_test("Func Invalid Name", "(a+b)(c)");
    run_test("Array Literal",
             "[10, 20, 30]",
             "[10 20 30]");
    run_test("Array with Math",
             "[1 + 2, a * b]",
             "[(+ 1 2) (* a b)]");
    run_test("Index Access",
             "my_arr[0]",
             "(index my_arr 0)");
    run_test("Math Index",
             "my_arr[i + 1]",
             "(index my_arr (+ i 1))");
    run_test("Nested Index Access",
             "matrix[i][j]",
             "(index (index matrix i) j)");
    run_test("Method Access",
             "obj.method",
             "(. obj method)");
    run_test("Method Call",
             "obj.method(10, 20)",
             "(call (. obj method) 10 20)");
    run_test("Method Call on Array Element",
             "my_arr[0].print()",
             "(call (. (index my_arr 0) print))");
    run_test("Var Declaration Simple",
             "int64 a = 10",
             "(var int64 a 10)");
    run_test("Var Declaration No Init",
             "string text",
             "(var string text)");
    run_test("Var Declaration Compound Type",
             "list<int64> my_arr = [1, 2, 3]",
             "(var list<int64> my_arr [1 2 3])");
    run_test("Var Declaration Math",
             "float res = a + b * 2",
             "(var float res (+ a (* b 2)))");


}


int main() {
    try {
        run_tests();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
