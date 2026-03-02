#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <string>
#include <set>
#include <list>
#include <chrono>

#include "../../include/compiler/compiler.h"
#include "../../include/compiler/codeconversion.h"
#include "../../include/notifier/notifier.h"

    enum class Parameters {
        Bracketing = 1,
        Spacing = 2,
        None = 0
    };
    enum class VarType {
        None = 0,
        Integer = 1,
        FloatingPoint = 2,
        String = 4,
        Boolean = 8
    };
    struct TypeBinder {
        std::string cppName;
        std::string pandacName;
        VarType varType;
        size_t convertibleTo;
        TypeBinder(std::string cppName, std::string pandacName, VarType varType, size_t convertibleTo)
            : cppName(cppName), pandacName(pandacName), varType(varType), convertibleTo(convertibleTo) {}
    };
    struct Variable {
        VarType varType;
        std::string name;
        size_t depth;
        Variable(std::string name, VarType varType, size_t depth)
            : name(name), varType(varType), depth(depth) {}
    };
    using ProcessingFunc = std::string(*)(std::vector<std::string>*, std::vector<TypeBinder>*);
    struct Keyword {
        ProcessingFunc processing;
        size_t params;
        std::string name;
        Keyword(std::string name, ProcessingFunc processing, size_t params = 0)
            : name(std::move(name)), processing(processing), params(params) {}
    };

    int Compiler::countIndentation(const std::string& line) {
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
    bool Compiler::isFileValid(const std::string& filePath) {
        if (filePath.length() < 8)
            return false;
        return filePath.rfind(".pandac") != std::string::npos
        && (filePath.find_first_of(" ") == std::string::npos)
        && (filePath.find('.') == filePath.rfind('.'));
    }

std::set<std::string> Compiler::CodeConvertionClass::cppLibrariesUsed{};
std::set<std::string> Compiler::CodeConvertionClass::pandaCLibrariesUsed{};
    int Compiler::run(std::string file) {
        auto start_time = std::chrono::high_resolution_clock::now();
        if(!isFileValid(file)) {
            Notifier::notifyError(ERROR_TYPE::FILE_NOT_FOUND);
            return 1;
        }
        Notifier::notifyInfo("Compiling file: " + file);

        std::vector<Keyword> keywords;
        keywords.emplace_back("if", &CodeConvertion::processIf, (size_t)Parameters::Bracketing);
        keywords.emplace_back("else", &CodeConvertion::processElse);
        keywords.emplace_back("return", &CodeConvertion::processReturn, (size_t)Parameters::Spacing);
        // keywords.emplace_back("for", &CodeConvertion::processFor);
        keywords.emplace_back("print", &CodeConvertion::processPrint, (size_t)Parameters::Bracketing);
        keywords.emplace_back("def", &CodeConvertion::processDef, (size_t)Parameters::Spacing | (size_t)Parameters::Bracketing);
        keywords.emplace_back("using", &CodeConvertion::processUsing, (size_t)Parameters::Spacing);
        keywords.emplace_back("", nullptr);

        std::vector<TypeBinder> typeBinders;
        typeBinders.emplace_back("int64_t", "int64", VarType::Integer, (size_t)VarType::Integer | (size_t)VarType::FloatingPoint | (size_t)VarType::String);
        typeBinders.emplace_back("int32_t", "int", VarType::Integer, (size_t)VarType::Integer | (size_t)VarType::FloatingPoint | (size_t)VarType::String);
        typeBinders.emplace_back("int8_t", "int8", VarType::Integer, (size_t)VarType::Integer | (size_t)VarType::FloatingPoint | (size_t)VarType::String);
        typeBinders.emplace_back("uint64_t", "uint64", VarType::Integer, (size_t)VarType::Integer | (size_t)VarType::FloatingPoint | (size_t)VarType::String);
        typeBinders.emplace_back("uint32_t", "uint", VarType::Integer, (size_t)VarType::Integer | (size_t)VarType::FloatingPoint | (size_t)VarType::String);
        typeBinders.emplace_back("uint8_t", "uint8", VarType::Integer, (size_t)VarType::Integer | (size_t)VarType::FloatingPoint | (size_t)VarType::String);
        typeBinders.emplace_back("double", "fl2", VarType::FloatingPoint,(size_t)VarType::FloatingPoint | (size_t)VarType::String);
        typeBinders.emplace_back("float", "fl1", VarType::FloatingPoint,(size_t)VarType::FloatingPoint | (size_t)VarType::String);
        typeBinders.emplace_back("std::string", "str", VarType::String, (size_t)VarType::String);
        typeBinders.emplace_back("bool", "bool", VarType::Boolean, 0);
        typeBinders.emplace_back("", "", VarType::None, 0);

        std::ifstream in(file);
        if (!in) {
            Notifier::notifyError(ERROR_TYPE::FILE_NOT_FOUND);
            return 1;
        }
        std::string mainCode = CodeConvertion::convert(in, keywords, typeBinders);


        std::ofstream out("pandaC.cpp");
        for (auto& item : CodeConvertion::cppLibrariesUsed)
            out << "#include <" << item << ">\n";
        std::string pandaClibraries = "";

        for (auto& item : CodeConvertion::pandaCLibrariesUsed) {
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
        int compileResult = std::system("g++ -std=c++23 pandaC.cpp -o pandaC_executable");
        if (compileResult != 0) {
            Notifier::notifyError(ERROR_TYPE::UNKNOWN_ERROR);
            return 1;
        }

        auto duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time);
        Notifier::notifyInfo("Compilation finished successfully in " + (duration.count() == 0 ? "<1" : std::to_string(duration.count())) + " seconds.");
        return 0;
    }