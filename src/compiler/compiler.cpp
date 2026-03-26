#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <string>
#include <set>
#include <filesystem>
#include <chrono>

#include "../../include/compiler/compiler.h"
#include "../../include/compiler/codeconvertion.h"
#include "../../include/notifier/notifier.h"

int countIndentation(const std::string &line) {
    int count = 0;
    for (const char& item : line) {
        if (item == ' ')
            ++count;
        else if (item == '\t')
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


std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        size_t first = token.find_first_not_of(" \t");
        size_t last = token.find_last_not_of(" \t");
        if (first != std::string::npos && last != std::string::npos)
            tokens.push_back(token.substr(first, (last - first + 1)));
        else if (first != std::string::npos)
            tokens.push_back(token.substr(first));
        else
            tokens.emplace_back("");
    }
    return tokens;
}

namespace {
    void loadLibraryConfig(const std::filesystem::path& confPath) {
        if (!std::filesystem::exists(confPath)) return;

        std::ifstream inConf(confPath);
        std::string confLine;
        bool parsingCppLibs = false;

        while (std::getline(inConf, confLine)) {
            if (confLine.find("CppLibraries = {") != std::string::npos) {
                parsingCppLibs = true;
                continue;
            }
            if (parsingCppLibs) {
                if (confLine.find('}') != std::string::npos) {
                    parsingCppLibs = false;
                    continue;
                }
                size_t first = confLine.find_first_not_of(" \t\r\n");
                if (first == std::string::npos) continue;
                size_t last = confLine.find_last_not_of(" \t\r\n");
                std::string libName = confLine.substr(first, (last - first + 1));
                if (!libName.empty()) {
                    CodeConvertion::cppLibrariesUsed.insert(libName);
                }
            }
        }
        inConf.close();
    }

    std::string loadLibrarySource(const std::string& item, bool log) {
        std::string code;
        std::filesystem::path libFolder = "../libraries/" + item;
        std::filesystem::path cppPath = libFolder / (item + ".cpp");
        std::filesystem::path confPath = libFolder / (item + ".conf");

        if (!std::filesystem::exists(cppPath))
            cppPath = "../libraries/" + item + ".cpp";

        code += "//START OF BLOCK: " + item + "\n\n";
        std::ifstream inLib(cppPath);
        if (inLib.is_open()) {
            std::string libLine;
            while (std::getline(inLib, libLine))
                code += libLine + "\n";
            inLib.close();
        }
        else if (log) Notifier::notifyInfo("Warning: Could not find source for library " + item);
        code += "//END OF BLOCK: " + item + "\n\n";

        loadLibraryConfig(confPath);
        return code;
    }
}

std::string Compiler::run(std::stringstream& input, bool log) {
    std::stringstream output;

    std::vector<Compiler::Keyword> keywords;
    std::vector<Compiler::TypeBinder> typeBinders;
    typeBinders.emplace_back("", "", Compiler::VarType::None, 0);

    std::string mainCode = CodeConvertion::convert(input, keywords, typeBinders);

    std::string pandaClibraries;
//Todo: reconsider library import algorithm
    std::vector<std::string> sortedLibs;
    if (CodeConvertion::pandaCLibrariesUsed.contains("pandaC")) {
        sortedLibs.emplace_back("pandaC");
    }
    for (const auto &item : CodeConvertion::pandaCLibrariesUsed) {
        if (item != "pandaC") {
            sortedLibs.push_back(item);
        }
    }

    for (auto &item: sortedLibs) {
        pandaClibraries += loadLibrarySource(item, log);
    }

    for (auto &item: CodeConvertion::cppLibrariesUsed)
        output << "#include <" << item << ">\n";

    output << pandaClibraries;
    output << mainCode;

    return output.str();
}

std::string Compiler::build(const std::string& code, const std::string& filePath, bool log) {
    if (filePath.empty()) {
        Notifier::notifyError(ERROR_TYPE::FILE_NOT_FOUND);
        throw std::runtime_error("File not found.");
    }

    std::string pathToOutput = filePath.substr(0, filePath.rfind('/')) + "/pandaC_build";
    ensureExists(pathToOutput);

    std::string filenameOnly = filePath.substr(filePath.rfind('/'));
    std::string baseName = filenameOnly.substr(0, filenameOnly.rfind('.'));
    std::string outputFile = pathToOutput + baseName;
    std::string cppFile = outputFile + ".cpp";

    std::ofstream out(cppFile);
    out << code;
    out.close();

    std::string cxx_compiler = "g++";
    if (const char* env_cxx = std::getenv("CXX")) {
        cxx_compiler = env_cxx;
    } else {
#ifdef __APPLE__
        cxx_compiler = "clang++";
#endif
    }
    std::string command = cxx_compiler + " -std=c++23 -O3 -w \"" + cppFile + "\" -o \"" + outputFile + "\"";

    if (log) {
        Notifier::notifyInfo("Invoking backend C++ compiler: " + cxx_compiler);
    }

    int result = std::system(command.c_str());
    if (result != 0) {
        if (log) Notifier::notifyError(ERROR_TYPE::UNKNOWN_ERROR);
        throw std::runtime_error("Backend C++ compilation failed with code " + std::to_string(result));
    }

    return outputFile;
}

void Compiler::execute(const std::string& file, const bool& log) {
#ifdef _WIN32
    FILE *pipe = _popen(outputFile.c_str(), "r");
#else
    FILE *pipe = popen(file.c_str(), "r");
#endif
    char buffer[1024];
    if (pipe) {
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            std::cout << buffer;
#ifdef _WIN32
        _pclose(pipe);
#else
        pclose(pipe);
#endif
    }
}