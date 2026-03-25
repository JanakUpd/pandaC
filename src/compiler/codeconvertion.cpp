#include "codeconvertion.h"
#include "lexer.h"
#include <iostream>

std::set<std::string> CodeConvertion::cppLibrariesUsed;
std::set<std::string> CodeConvertion::pandaCLibrariesUsed;

std::string CodeConvertion::convert(std::istream& in,
                                    const std::vector<Compiler::Keyword>& keywords,
                                    std::vector<Compiler::TypeBinder>& typeBinders) {
    std::string fullCode((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    try {
        Lexer lexer;
        Expression ast = lexer.fromString(fullCode);
        // std::cout << "DEBUG: TOKENS (" << lexer.tokens.size() << "): " << std::endl;
        // for (const auto& t : lexer.tokens) {
        //     std::cout << "[" << (t.type == TokenType::Newline ? "NEWLINE" : t.lexeme) << "] ";
        //     if(t.type == TokenType::Newline) std::cout << std::endl;
        // }
        // std::cout << std::endl << "--- END TOKENS ---" << std::endl;
        if (std::holds_alternative<Program>(ast.value)) {
            const auto& prog = std::get<Program>(ast.value);
            for (const auto& stmt : prog.statements) {
                if (std::holds_alternative<UsingStatement>(stmt->value)) {
                    const auto& use_stmt = std::get<UsingStatement>(stmt->value);
                    pandaCLibrariesUsed.insert(use_stmt.lib_name);
                }
            }
        }

        return Lexer::toCppString(ast, 0, &typeBinders);
    } catch (const std::exception& e) {
        std::cerr << "Compilation failed during syntax analysis.\n";
        std::cerr << "Error: " << e.what() << std::endl;
        throw;
    }
}
