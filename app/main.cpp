#include <ostream>
#include <string>
#include "../include/compiler/compiler.h"

std::string findSpecifier(int argc, char** argv, const std::string& specifier)
{
    std::string res;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.find(specifier, 0) == 0) {
            res = arg.substr(specifier.length());
            return res;
        }
    }
    return "";
}

bool containsSpecifier(int argc, char** argv, const std::string& specifier)
{
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.find(specifier, 0) == 0) {
            return true;
        }
    }
    return false;
}

int main(int argc, char** argv) {
    Compiler compiler;
    std::string filePath;
    if (containsSpecifier(argc, argv, "-f"))
        filePath = findSpecifier(argc, argv, "-f=");
    else
        filePath = "main.pc";
    return compiler.run(filePath, !containsSpecifier(argc, argv, "--no-execution"), containsSpecifier(argc, argv, "--debug"));
}