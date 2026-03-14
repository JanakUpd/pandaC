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
            tokens.push_back("");
    }
    return tokens;
}

int Compiler::run(std::string file, bool execute, bool log) {
    auto start_time = std::chrono::high_resolution_clock::now();
    if (!isFileValid(file)) {
        if (log) Notifier::notifyError(ERROR_TYPE::FILE_NOT_FOUND);
        return 1;
    }
    if (log) Notifier::notifyInfo("Compiling file: " + file);

    CodeConvertion::cppLibrariesUsed.clear();
    CodeConvertion::pandaCLibrariesUsed.clear();

    std::vector<Compiler::Keyword> keywords;
    std::vector<Compiler::TypeBinder> typeBinders;

    std::ifstream keywordsConfig("../config/default.conf");

    if (keywordsConfig.is_open()) {
        std::string line;
        bool parsingCommands = false;
        bool parsingTypeBinders = false;

        while (std::getline(keywordsConfig, line)) {
            if (line.find("Commands = {") != std::string::npos) {
                parsingCommands = true;
                continue;
            }
            if (line.find("TypeBinders = {") != std::string::npos) {
                parsingTypeBinders = true;
                continue;
            }
            if (line.find("}") != std::string::npos && line.find("{") == std::string::npos) {
                parsingCommands = false;
                parsingTypeBinders = false;
                continue;
            }

            size_t openBrace = line.find('{');
            size_t closeBrace = line.rfind('}');

            if (openBrace == std::string::npos || closeBrace == std::string::npos || closeBrace <= openBrace)
                continue;

            std::string content = line.substr(openBrace + 1, closeBrace - openBrace - 1);
            std::vector<std::string> parts = split(content, '@');

            if (parsingCommands) {
                if (parts.size() >= 3) {
                    std::string name = parts[0];
                    std::string patternStr = parts[1];
                    std::string funcStr = parts[2];
                    size_t flags = 0;
                    if (parts.size() >= 4) { try { flags = std::stoul(parts[3]); } catch(...) { flags = 0; } }

                    if (name.empty() || funcStr.empty()) continue;
                    if (name == "print") CodeConvertion::cppLibrariesUsed.insert("iostream");

                    bool found = false;
                    for (auto &existing : keywords) {
                        if (existing.name == name) {
                            existing.maps.push_back(patternStr + "@@@" + funcStr);
                            found = true;
                            break;
                        }
                    }
                    if (!found) keywords.emplace_back(name, std::vector<std::string>{patternStr + "@@@" + funcStr}, flags);
                }
            }
            else if (parsingTypeBinders) {
                if (parts.size() >= 2) {
                    std::string cppName = parts[0];
                    std::string pandaName = parts[1];
                    int varTypeInt = static_cast<int>(Compiler::VarType::None);
                    size_t flags = 0;
                    if (parts.size() > 2) { try { varTypeInt = std::stoi(parts[2]); } catch(...) {} }
                    if (parts.size() > 3) { try { flags = std::stoul(parts[3]); } catch(...) {} }
                    typeBinders.emplace_back(cppName, pandaName, static_cast<Compiler::VarType>(varTypeInt), flags);
                }
            }
        }
        keywordsConfig.close();
    }
    keywords.emplace_back("", std::vector<std::string>{}, 0);
    typeBinders.emplace_back("", "", Compiler::VarType::None, 0);

    std::ifstream in(file);
    if (!in) {
        if (log) Notifier::notifyError(ERROR_TYPE::FILE_NOT_FOUND);
        return 1;
    }
    std::string mainCode = CodeConvertion::convert(in, keywords, typeBinders);
    in.close();

    std::string pandaClibraries = "";

    std::vector<std::string> sortedLibs;
    if (CodeConvertion::pandaCLibrariesUsed.count("pandaC")) {
        sortedLibs.push_back("pandaC");
    }
    for (const auto &item : CodeConvertion::pandaCLibrariesUsed) {
        if (item != "pandaC") {
            sortedLibs.push_back(item);
        }
    }

    for (auto &item: sortedLibs) {
        std::filesystem::path libFolder = "../libraries/" + item;
        std::filesystem::path cppPath = libFolder / (item + ".cpp");
        std::filesystem::path confPath = libFolder / (item + ".conf");

        if (!std::filesystem::exists(cppPath)) {
            cppPath = "../libraries/" + item + ".cpp";
        }

        pandaClibraries += "//START OF BLOCK: " + item + "\n\n";
        std::ifstream inLib(cppPath);
        if (inLib.is_open()) {
            std::string libLine;
            while (std::getline(inLib, libLine))
                pandaClibraries += libLine + "\n";
            inLib.close();
        } else {
            if (log) Notifier::notifyInfo("Warning: Could not find source for library " + item);
        }
        pandaClibraries += "//END OF BLOCK: " + item + "\n\n";

        if (std::filesystem::exists(confPath)) {
            std::ifstream inConf(confPath);
            std::string confLine;
            bool parsingCppLibs = false;
            while (std::getline(inConf, confLine)) {
                if (confLine.find("CppLibraries = {") != std::string::npos) {
                    parsingCppLibs = true;
                    continue;
                }
                if (parsingCppLibs) {
                    if (confLine.find("}") != std::string::npos) {
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
    }

    std::string pathToOutput = file.substr(0, file.rfind('/')) + "/pandaC_build";
    ensureExists(pathToOutput);
    std::string filenameOnly = file.substr(file.rfind('/'));
    std::string baseName = filenameOnly.substr(0, filenameOnly.rfind('.'));
    std::string outputFile = pathToOutput + baseName;
    std::ofstream out(outputFile + ".cpp");

    for (auto &item: CodeConvertion::cppLibrariesUsed)
        out << "#include <" << item << ">\n";

    out << pandaClibraries;
    out << mainCode;
    out.close();

    if (std::system(("g++ -std=c++23 " + outputFile + ".cpp" + " -o " + outputFile).c_str()) != 0) {
        if (log) Notifier::notifyError(ERROR_TYPE::UNKNOWN_ERROR);
        return 1;
    }

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start_time);
    if (log)
        Notifier::notifyInfo(
            "Compilation finished successfully in " + (duration.count() == 0 ? "<1" : std::to_string(duration.count()))
            + " miliseconds.");

    start_time = std::chrono::high_resolution_clock::now();
    if (execute) {
#ifdef _WIN32
        FILE *pipe = _popen(outputFile.c_str(), "r");
#else
        FILE *pipe = popen(outputFile.c_str(), "r");
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
            duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start_time);
        Notifier::notifyInfo("Program executed in " + (duration.count() == 0 ? "<1" : std::to_string(duration.count())) + " miliseconds.");
    }
    return 0;
}
