#include <string>

#include "preprocessor.h"
#include "compiler.h"
#include "notifier.h"

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
    std::string filePath;
    if (containsSpecifier(argc, argv, "-f"))
        filePath = findSpecifier(argc, argv, "-f=");
    else
        filePath = "main.pc";

    bool log = containsSpecifier(argc, argv, "--debug");
    if (log) Notifier::notifyInfo("Starting compilation of " + filePath);

    if (log)
        Notifier::notifyInfo("Preprocessing...");
    auto start_time = std::chrono::high_resolution_clock::now();
    std::stringstream ss = Preprocessor::run(filePath, log);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time);
    if (log)
        Notifier::notifyInfo("Preprocessing finished successfully in " + (duration.count() == 0 ? "<1" : std::to_string(duration.count())) + " miliseconds.");

    if (log)
        Notifier::notifyInfo("Compiling...");
    start_time = std::chrono::high_resolution_clock::now();
    std::string cppTranslatedCode = Compiler::run(ss, !containsSpecifier(argc, argv, "--no-execution"), log);
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time);
    if (log)
        Notifier::notifyInfo("Compilation finished successfully in " + (duration.count() == 0 ? "<1" : std::to_string(duration.count())) + " miliseconds.");
    Compiler::build(cppTranslatedCode, filePath, !containsSpecifier(argc, argv, "--no-execution"), log);
}