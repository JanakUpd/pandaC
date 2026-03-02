#include <string>
#include <set>
#include "../include/compiler/compiler.h"
#include "../include/notifier/notifier.h"

std::string findSpecifier(int argc, char** argv, const std::string& specifier)
{
    std::string res;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.rfind(specifier, 0) == 0) {
            res = arg.substr(3);
            return res;
        }
    }
    return "";
}



int main(int argc, char** argv) {
    Compiler compiler;
    std::string filePath;
    if (argc != 1)
        filePath = findSpecifier(argc, argv, "-f=");
    else
        filePath = "main.pandac";
    compiler.run(filePath);
}