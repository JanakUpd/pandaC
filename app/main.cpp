#include <iostream>
#include <string>

#include "preprocessor.h"
#include "compiler.h"
#include "notifier.h"
#include "clock.h"

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
    //Extract filepath from arguments
    std::string filePath;
    if (containsSpecifier(argc, argv, "-f"))
        filePath = findSpecifier(argc, argv, "-f=");
    else
        filePath = "main.pc";

    bool log = containsSpecifier(argc, argv, "--debug");
    if (log) Notifier::notifyInfo("Starting compilation of " + filePath);

    //Preprocessing: pandac code insertion and comment removal
    if (log)
        Notifier::notifyInfo("Preprocessing...");
    clockTimer timer = clockTimer();
    uint64_t duration;
    timer.start();
    std::stringstream ss = Preprocessor::run(filePath, log);
    duration = timer.getMicroseconds();
    if (log)
        Notifier::notifyInfo("Preprocessing finished successfully in " + (duration == 0 ? "<1" : std::to_string(duration)) + " microseconds.");

    //Compiling: converting pandac code into c++
    if (log)
        Notifier::notifyInfo("Compiling...");
    timer.start();
    std::string cppTranslatedCode = Compiler::run(ss, log);
    duration = timer.getMiliseconds();
    if (log)
        Notifier::notifyInfo("Compilation finished successfully in " + (duration == 0 ? "<1" : std::to_string(duration)) + " miliseconds.");

    //Building: building the cpp code into executable via gcc or alternative
    timer.start();
    std::string exeFile = Compiler::build(cppTranslatedCode, filePath, log);
    duration = timer.getMiliseconds();
    Notifier::notifyInfo("Program build by backend in " + (duration == 0 ? "<1" : std::to_string(duration)) + " miliseconds.");

    //Execution: start compiled file
    if (!containsSpecifier(argc, argv, "--no-execution")) {
        timer.start();
        Compiler::execute(exeFile, log);
        duration = timer.getMiliseconds();
        Notifier::notifyInfo("Program executed in " + (duration == 0 ? "<1" : std::to_string(duration)) + " miliseconds.");
    }
}