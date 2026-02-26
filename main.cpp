#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <string>
#include <set>
#include <list>
#include <chrono>


enum class ERROR_TYPE {
    FILE_NOT_FOUND,
    SYNTAX_ERROR,
    UNKNOWN_ERROR
};

class Notifier{
public:
    //red messages
    static void notifyError(ERROR_TYPE errorType) {
        std::cerr << "\033[1;31m";
        std::cerr << "[ERR COMPILATION]: ";
        switch(errorType) {
            case ERROR_TYPE::FILE_NOT_FOUND:
                std::cerr << "file not found" << std::endl;
                break;
            case ERROR_TYPE::SYNTAX_ERROR:
                std::cerr << "syntax error" << std::endl;
                break;
            case ERROR_TYPE::UNKNOWN_ERROR:
                std::cerr << "unknown error occurred" << std::endl;
                break;
        }
        std::cerr << "\033[0m";
    }
    //yellow messages
    static void notifyInfo(const std::string& info) {
        std::cout << "\033[1;34m";
        std::cout << "[INFO]: " << info << std::endl;
        std::cout << "\033[0m";
    }
};

class Compiler {
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
    class CodeConvertionClass {
    private:
    static std::vector<std::string> parseArguments(const std::string& argsStr, size_t paramFlags) {
        std::vector<std::string> params;
        bool useSpacing = (paramFlags & static_cast<size_t>(Parameters::Spacing)) != 0;
        bool useBracketing = (paramFlags & static_cast<size_t>(Parameters::Bracketing)) != 0;

        std::string buffer;
        size_t depth = 0;
        bool inBrackets = false;

        for (char c : argsStr) {
            if (useBracketing && c == '(') {
                if (depth == 0) {
                    if (useSpacing && !buffer.empty()) {
                        params.push_back(buffer);
                        buffer.clear();
                    }
                    inBrackets = true;
                }
                else {
                    buffer += c;
                }
                ++depth;
            }
            else if (useBracketing && c == ')') {
                depth--;
                if (depth == 0) {
                    params.push_back(buffer);
                    buffer.clear();
                    inBrackets = false;
                }
                else {
                    buffer += c;
                }
            }
            else if (inBrackets) {
                buffer += c;
            }
            else if (useSpacing) {
                if (std::isspace(c) || c == ':') {
                    if (!buffer.empty()) {
                        params.push_back(buffer);
                        buffer.clear();
                    }
                }
                else {
                    buffer += c;
                }
            }
        }
        if (!buffer.empty())
            params.push_back(buffer);

        return params;
    }

    private:
        static std::pair<size_t, std::string> parseLine(const std::string& line) {
            size_t indentCount = 0;
            size_t i = 0;
            while (i < line.size()) {
                if (line[i] == ' ') { indentCount += 1; i++; }
                else if (line[i] == '\t') { indentCount += 4; i++; }
                else break;
            }
            std::string trimmed = line.substr(i);
            while (!trimmed.empty() && std::isspace(trimmed.back())) {
                trimmed.pop_back();
            }
            return {indentCount, trimmed};
        }

        static std::string adjustBraces(size_t currentIndent, size_t newIndent) {
            std::string result = "";
            for (size_t i = currentIndent; i > newIndent; i -= 4) {
                result += createIndentation(i - 4) + "}\n";
            }
            return result;
        }

    private:
        static std::string createIndentation(size_t ind) {
            std::string res;
            for (size_t i = 0; i < ind; ++i)
                res += ' ';
            return res;
        }
        static const Keyword* findKeyword(const std::string& line, const std::vector<Keyword>& keywords) {
            for (const auto& item : keywords) {
                if (!item.name.empty() && line.starts_with(item.name)) {
                    return &item;
                }
            }
            return nullptr;
        }
        static const TypeBinder& findTypeBinder(std::string& s, const std::vector<TypeBinder>& typeBinders) {
            for (auto& item : typeBinders)
                if (s.size() >= item.pandacName.size() && s.substr(s.size() - item.pandacName.size(), item.pandacName.size()) == item.pandacName)
                    return item;
            return typeBinders.back();
        }
    public:
        static std::string convert(std::ifstream& in, const std::vector<Keyword>& keywords, std::vector<TypeBinder>& typeBinders) {
            std::string finalCppCode;
            std::string line;
            size_t currentIndent = 0;

            while (std::getline(in, line)) {
                if (line.empty()) continue;
                auto [lineIndent, command] = parseLine(line);
                if (command.empty()) continue;
                if (lineIndent < currentIndent)
                    finalCppCode += adjustBraces(currentIndent, lineIndent);

                auto* keyword = findKeyword(command, keywords);
                if (keyword) {
                    std::string argsStr = command.substr(keyword->name.size());
                    std::vector<std::string> params = parseArguments(argsStr, keyword->params);
                    finalCppCode += createIndentation(lineIndent);
                    finalCppCode += keyword->processing(&params, &typeBinders);
                }
                else {
                    finalCppCode += createIndentation(lineIndent) + command + ";\n";
                }

                currentIndent = lineIndent;
            }
            finalCppCode += adjustBraces(currentIndent, 0);
            return finalCppCode;
        }
        static std::set<std::string> cppLibrariesUsed;
        static std::string translateArgs(const std::string& rawArgs, const std::vector<TypeBinder>* typeBinders) {
            if (rawArgs.empty()) return "";

            std::string result = "";
            std::string currentArg = "";

            std::string argsToParse = rawArgs + ",";
            bool first = true;

            for (char c : argsToParse) {
                if (c == ',') {
                    size_t start = currentArg.find_first_not_of(" \t");
                    if (start != std::string::npos) {
                        currentArg = currentArg.substr(start);
                        size_t spacePos = currentArg.find_first_of(" \t");
                        if (spacePos != std::string::npos) {
                            std::string pandaType = currentArg.substr(0, spacePos);

                            size_t namePos = currentArg.find_first_not_of(" \t", spacePos);
                            std::string varName = (namePos != std::string::npos) ? currentArg.substr(namePos) : "";

                            std::string cppType = pandaType;
                            for (const auto& tb : *typeBinders) {
                                if (tb.pandacName == pandaType) {
                                    cppType = tb.cppName;
                                    break;
                                }
                            }

                            if (!first) result += ", ";
                            result += cppType + " " + varName;
                            first = false;
                        } else {
                            if (!first) result += ", ";
                            result += currentArg;
                            first = false;
                        }
                    }
                    currentArg.clear();
                } else {
                    currentArg += c;
                }
            }

            return result;
        }

        static std::string processDef(std::vector<std::string>* params, std::vector<TypeBinder>* typeBinders) {
            if (params->empty()) return "";

            // Handle "main"
            for (const auto& p : *params) {
                if (p == "main") return "int main() {\n";
            }

            std::string returnType = "void";
            std::string funcName = "";
            std::string rawArgs = "";

            if (params->size() == 1) {
                funcName = (*params)[0];
            }
            else if (params->size() == 2) {
                bool hasType = false;
                for (const auto& item : *typeBinders) {
                    if ((*params)[0] == item.pandacName) {
                        returnType = item.cppName;
                        hasType = true;
                        break;
                    }
                }
                if (hasType)
                    funcName = (*params)[1];
                else {
                    funcName = (*params)[0];
                    rawArgs = (*params)[1];
                }
            }
            else if (params->size() >= 3) {
                for (const auto& item : *typeBinders) {
                    if ((*params)[0] == item.pandacName) {
                        returnType = item.cppName;
                        break;
                    }
                }
                funcName = (*params)[1];
                rawArgs = (*params)[2];
            }
            std::string translatedArgs = translateArgs(rawArgs, typeBinders);
            return returnType + " " + funcName + "(" + translatedArgs + ") {\n";
        }


        static std::string processReturn(std::vector<std::string>* params,  std::vector<TypeBinder>* typeBinders) {
            switch (params->size()) {
                case 1: return "return " + (*params)[0] + ";\n";
                default: return "return;\n";
            }
        }
        static std::string processIf(std::vector<std::string>* params, std::vector<TypeBinder>* typeBinders) {
            return "if (" + (*params)[0] + ") {\n";
        }
        static std::string processElse(std::vector<std::string>* params, std::vector<TypeBinder>* typeBinders) {
            return "else {\n";
        }
        static std::string processFor(std::vector<std::string>* params, std::vector<TypeBinder>* typeBinders) {
            return "";
        };
        static std::string processPrint(std::vector<std::string>* params, std::vector<TypeBinder>* typeBinders) {
            cppLibrariesUsed.emplace("iostream");
            switch ((*params).size()) {
                case 1:
                    return "std::cout << " + (*params)[0] + " << std::endl;\n";
                case 2:
                    return "std::cout << " + (*params)[0] + " << " + (*params)[1] + ";\n";
                default:
                    Notifier::notifyError(ERROR_TYPE::SYNTAX_ERROR);
                    return "";
            }
        }
    };
    int countIndentation(const std::string& line) {
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
    bool isFileValid(const std::string& filePath) {
        if (filePath.length() < 8) 
            return false;
        return filePath.rfind(".pandac") != std::string::npos
        && (filePath.find_first_of(" ") == std::string::npos)
        && (filePath.find('.') == filePath.rfind('.'));
    }
public:
    int run(std::string file) {
        auto start_time = std::chrono::high_resolution_clock::now();
        if(!isFileValid(file)) {
            Notifier::notifyError(ERROR_TYPE::FILE_NOT_FOUND);
            return 1;
        }
        Notifier::notifyInfo("Compiling file: " + file);

        std::vector<Keyword> keywords;
        keywords.emplace_back("if", &CodeConvertionClass::processIf, (size_t)Parameters::Bracketing);
        keywords.emplace_back("else", &CodeConvertionClass::processElse);
        keywords.emplace_back("return", &CodeConvertionClass::processReturn, (size_t)Parameters::Spacing);
        // keywords.emplace_back("for", &CodeConvertionClass::processFor);
        keywords.emplace_back("print", &CodeConvertionClass::processPrint, (size_t)Parameters::Bracketing);
        keywords.emplace_back("def", &CodeConvertionClass::processDef, (size_t)Parameters::Spacing | (size_t)Parameters::Bracketing);
        keywords.emplace_back("", nullptr);

        std::vector<TypeBinder> typeBinders;
        typeBinders.emplace_back("int32_t", "int", VarType::Integer, (size_t)VarType::Integer | (size_t)VarType::FloatingPoint | (size_t)VarType::String);
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
        std::string mainCode = CodeConvertionClass::convert(in, keywords, typeBinders);


        std::ofstream out("pandaC.cpp");
        for (auto& item : CodeConvertionClass::cppLibrariesUsed)
            out << "#include <" << item << ">\n";
        out << mainCode;
        out.close();
        in.close();
        int compileResult = std::system("g++ pandaC.cpp -o pandaC_executable");
        if (compileResult != 0) {
            Notifier::notifyError(ERROR_TYPE::UNKNOWN_ERROR);
            return 1;
        }

        auto duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time);
        Notifier::notifyInfo("Compilation finished successfully in " + (duration.count() == 0 ? "<1" : std::to_string(duration.count())) + " seconds.");
        return 0;
    }
};

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

std::set<std::string> Compiler::CodeConvertionClass::cppLibrariesUsed{};
int main(int argc, char** argv) {
    Compiler compiler;
    std::string filePath;
    if (argc != 1)
        filePath = findSpecifier(argc, argv, "-f=");
    else
        filePath = "main.pandac";
    compiler.run(filePath);
}