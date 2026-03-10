#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <string>
#include <set>
#include <list>
#include <filesystem>
#include <chrono>

#include "../../include/compiler/compiler.h"
#include "../../include/compiler/codeconvertion.h"
#include "../../include/notifier/notifier.h"

int Compiler::countIndentation(const std::string &line) {
    int count = 0;
    for (auto it = line.begin(); it != line.end(); ++it) {
        if (*it == ' ')
            ++count;
        else if (*it == '\t')
            count += 3;
        else
            return count;
    }
    return count;
}

void ensureExists(const std::string& str) {
    std::filesystem::path file = str;
    std::filesystem::create_directory(file);
}

bool Compiler::isFileValid(const std::string &filePath) {
    if (filePath.length() < 4)
        return false;
    return filePath.rfind(".pc") != std::string::npos
           && (filePath.find('.') == filePath.rfind('.'));
}

std::set<std::string> Compiler::CodeConvertionClass::cppLibrariesUsed{};
std::set<std::string> Compiler::CodeConvertionClass::pandaCLibrariesUsed{};

int Compiler::run(std::string file, bool execute, bool log) {
    auto start_time = std::chrono::high_resolution_clock::now();
    if (!isFileValid(file)) {
        if (log) Notifier::notifyError(ERROR_TYPE::FILE_NOT_FOUND);
        return 1;
    }
    if (log) Notifier::notifyInfo("Compiling file: " + file);

    std::vector<Compiler::Keyword> keywords;
    keywords.emplace_back("if", &CodeConvertion::processIf, (size_t) Compiler::Parameters::Bracketing);
    keywords.emplace_back("else", &CodeConvertion::processElse);
    keywords.emplace_back("return", &CodeConvertion::processReturn, (size_t) Compiler::Parameters::Spacing);
    // keywords.emplace_back("for", &CodeConvertion::processFor);
    keywords.emplace_back("print", &CodeConvertion::processPrint, (size_t) Compiler::Parameters::Bracketing);
    keywords.emplace_back("def", &CodeConvertion::processDef,
                          (size_t) Compiler::Parameters::Spacing | (size_t) Compiler::Parameters::Bracketing);
    keywords.emplace_back("using", &CodeConvertion::processUsing, (size_t) Compiler::Parameters::Spacing);
    keywords.emplace_back("", nullptr);

    std::vector<Compiler::TypeBinder> typeBinders;
    typeBinders.emplace_back("int64_t", "int64", Compiler::VarType::Integer,
                             (size_t) Compiler::VarType::Integer | (size_t) Compiler::VarType::FloatingPoint | (size_t) Compiler::VarType::String);
    typeBinders.emplace_back("int32_t", "int", Compiler::VarType::Integer,
                             (size_t) Compiler::VarType::Integer | (size_t) Compiler::VarType::FloatingPoint | (size_t) Compiler::VarType::String);
    typeBinders.emplace_back("int8_t", "int8", Compiler::VarType::Integer,
                             (size_t) Compiler::VarType::Integer | (size_t) Compiler::VarType::FloatingPoint | (size_t) Compiler::VarType::String);
    typeBinders.emplace_back("uint64_t", "uint64", Compiler::VarType::Integer,
                             (size_t) Compiler::VarType::Integer | (size_t) Compiler::VarType::FloatingPoint | (size_t) Compiler::VarType::String);
    typeBinders.emplace_back("uint32_t", "uint", Compiler::VarType::Integer,
                             (size_t) Compiler::VarType::Integer | (size_t) Compiler::VarType::FloatingPoint | (size_t) Compiler::VarType::String);
    typeBinders.emplace_back("uint8_t", "uint8", Compiler::VarType::Integer,
                             (size_t) Compiler::VarType::Integer | (size_t) Compiler::VarType::FloatingPoint | (size_t) Compiler::VarType::String);
    typeBinders.emplace_back("double", "fl2", Compiler::VarType::FloatingPoint,
                             (size_t) Compiler::VarType::FloatingPoint | (size_t) Compiler::VarType::String);
    typeBinders.emplace_back("float", "fl1", Compiler::VarType::FloatingPoint,
                             (size_t) Compiler::VarType::FloatingPoint | (size_t) Compiler::VarType::String);
    typeBinders.emplace_back("std::string", "str", Compiler::VarType::String, (size_t) Compiler::VarType::String);
    typeBinders.emplace_back("bool", "bool", Compiler::VarType::Boolean, 0);
    typeBinders.emplace_back("", "", Compiler::VarType::None, 0);

    std::ifstream in(file);
    if (!in) {
        if (log) Notifier::notifyError(ERROR_TYPE::FILE_NOT_FOUND);
        return 1;
    }
    std::string mainCode = CodeConvertion::convert(in, keywords, typeBinders);

    std::filesystem::path dir = file.parent_path();
    std::string pathToOutput = file.substr(0, file.rfind('/')) + "/pandaC_build";
    ensureExists(pathToOutput);
    std::string filenameOnly = file.substr(file.rfind('/'));
    std::string baseName = filenameOnly.substr(0, filenameOnly.rfind('.'));
    std::string outputFile = pathToOutput + baseName;
    std::ofstream out(outputFile + ".cpp");

    for (auto &item: CodeConvertion::cppLibrariesUsed)
        out << "#include <" << item << ">\n";
    std::string pandaClibraries = "";

    for (auto &item: CodeConvertion::pandaCLibrariesUsed) {
        pandaClibraries += "//START OF BLOCK: " + item + "\n\n";
        std::ifstream in("../libraries/" + item + ".cpp");
        std::string line;
        while (std::getline(in, line))
            pandaClibraries += line + "\n";
        pandaClibraries += "//END OF BLOCK: " + item + "\n\n";
    }
    out << pandaClibraries;
    out << mainCode;
    out.close();
    in.close();
    if (std::system(("g++ -std=c++23 " + outputFile + ".cpp" + " -o " + outputFile).c_str()) != 0) {
        if (log) Notifier::notifyError(ERROR_TYPE::UNKNOWN_ERROR);
        return 1;
    }

    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::high_resolution_clock::now() - start_time);
    if (log)
        Notifier::notifyInfo(
            "Compilation finished successfully in " + (duration.count() == 0 ? "<1" : std::to_string(duration.count()))
            +
            " seconds.");
    if (execute) {
#ifdef _WIN32
        FILE *pipe = _popen(outputFile, "r");
#else
        FILE *pipe = popen(outputFile.c_str(), "r");
#endif
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            std::cout << buffer;

#ifdef _WIN32
        _pclose(pipe);
#else
        pclose(pipe);
#endif
    }
    return 0;
}