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
        static std::string createIndentation(size_t ind) {
            std::string res;
            for (size_t i = 0; i < ind; ++i)
                res += ' ';
            return res;
        }
        static const Keyword& findKeyword(std::string& s, const std::vector<Keyword>& keywords) {
            for (auto& item : keywords)
                // if (s.size() >= item.name.size() && s.substr(s.size()-item.name.size(), item.name.size()) == item.name)
                //     return item;
                if (s.ends_with(item.name))
                    return item;
            return keywords.back();
        }
        static const TypeBinder& findTypeBinder(std::string& s, const std::vector<TypeBinder>& typeBinders) {
            for (auto& item : typeBinders)
                if (s.size() >= item.pandacName.size() && s.substr(s.size() - item.pandacName.size(), item.pandacName.size()) == item.pandacName)
                    return item;
            return typeBinders.back();
        }
    public:
        static std::string convert(std::ifstream& in, const std::vector<Keyword>& keywords, std::vector<TypeBinder>& typeBinders) {
            std::list<Variable> variables;
            std::string content, currContent;
            char c, prevC;
            std::string line;
            bool indFlag = true;
            bool stringFlag = false;
            bool lineConsumed = false;
            std::string cnstString = "";
            size_t indCount = 0, prevIndCount = 0;
            while (in.get(c)) {
                if (!indFlag) {
                    if (prevC != '\\' && (c == '\'' || c == '"')) {
                        if (stringFlag) {}
                        stringFlag = !stringFlag;
                        cnstString += '"'; //to unify string and char
                        //cnstString += c; //not to unify
                        if (!stringFlag)
                            currContent += cnstString;
                    }
                    else if (stringFlag) {
                        cnstString += c;
                    }
                    else if (c != '\n' ) {
                        line += c;
                        auto& keyword = findKeyword(line, keywords);
                        if (&keyword != &keywords.back() && keyword.name != "") {
                            if (keyword.params == 0)
                                currContent += keyword.processing(nullptr, &typeBinders);
                            if ((keyword.params & (size_t)Parameters::Spacing) != 0) {
                                std::vector<std::string> params;
                                std::string buffer = "";
                                while (in.get(c)) {
                                    if (c == '\n') {
                                        params.push_back(buffer);
                                        currContent += keyword.processing(&params, &typeBinders);
                                        currContent = createIndentation(indCount) + currContent;
                                        if (indCount < prevIndCount) {
                                            std::string buffer = "";
                                            for (size_t i = indCount; i < prevIndCount; i+=4)
                                                buffer += createIndentation(i - indCount) + "}\n";
                                            currContent = createIndentation(indCount) + buffer + currContent;
                                        }
                                        prevIndCount = indCount;
                                        indCount = 0;
                                        indFlag = true;
                                        line = "";
                                        content += currContent;
                                        currContent = "";
                                        lineConsumed = false;
                                        break;
                                    }
                                    if (c == ':' || c == '(' || c == ')') {
                                        params.push_back(buffer);
                                        currContent += keyword.processing(&params, &typeBinders);
                                        break;
                                    }
                                    if (c != ' ')
                                        buffer += c;
                                    else if (buffer != ""){
                                        params.push_back(buffer);
                                        buffer = "";
                                    }
                                    prevC = c;
                                }
                            }
                            if ((keyword.params & (size_t)Parameters::Bracketing) != 0) {
                                std::vector<std::string> params;
                                std::string buffer;
                                int depth = 0;
                                bool started = false;
                                while (in.get(c)) {
                                    if (c == '(') {
                                        ++depth;
                                        started = true;
                                        if (depth > 1)
                                            buffer += c;
                                    }
                                    else if (c == ')') {
                                        --depth;
                                        if (depth == 0) {
                                            params.push_back(buffer);
                                            break;
                                        }
                                        buffer += c;
                                    }
                                    else if (started)
                                        buffer += c;
                                    prevC = c;
                                }
                                currContent += keyword.processing(&params, &typeBinders);
                            }

                            lineConsumed = true;
                        }
                        //TODO: PLACE IF FOR TYPES :))
                        // if () {
                        //
                        // }
                    }
                    else {
                        currContent = createIndentation(indCount) + currContent;
                        if (indCount < prevIndCount) {
                            std::string buffer = "";
                            for (size_t i = indCount; i < prevIndCount; i+=4)
                                buffer += createIndentation(i - indCount) + "}\n";
                            currContent = createIndentation(indCount) + buffer + currContent;
                        }
                        prevIndCount = indCount;
                        indCount = 0;
                        indFlag = true;
                        lineConsumed = false;
                        line = "";
                        content += currContent;
                        currContent = "";
                    }
                }
                else if (c == ' ')
                    ++indCount;
                else if (c == '\t')
                    indCount += 4;
                else {
                    indFlag = false;
                    line +=c;
                }
                prevC = c;
            }
            if (line != "") {
                if (!lineConsumed) {
                    auto& keyword = findKeyword(line, keywords);
                    if (keyword.name != "") {
                        if (keyword.params == 0) {
                            currContent += keyword.processing(nullptr, &typeBinders);
                        }
                    }
                }
                currContent = createIndentation(indCount) + currContent;
                if (indCount < prevIndCount) {
                    std::string buffer = "";
                    for (size_t i = indCount; i < prevIndCount; i+=4)
                        buffer += createIndentation(i - indCount) + "}\n";
                    currContent = createIndentation(indCount) + buffer + currContent;
                }
                content += currContent;
                currContent = "";
                lineConsumed = false;
            }
            size_t ind = std::max(indCount, prevIndCount);
            if (ind > 0) {
                std::string buffer = "";
                for (size_t i = 0; i < ind; i+=4)
                    buffer = createIndentation(i) + "}\n" + buffer;
                content += buffer;
            }
            return content;
        }
        static std::set<std::string> cppLibrariesUsed;
        static std::string processDef(std::vector<std::string>* params, std::vector<TypeBinder>* typeBinders) {
            if (params->back().find("main") !=  std::string::npos)
                return "int main() {\n";
            switch (params->size()) {
                case 1:
                    return "void " + (*params)[0] + "() {\n";
                case 2: {
                    std::string type = "";
                    for (auto& item : *typeBinders)
                        if ((*params)[0] == item.pandacName) {
                            type = item.cppName;
                            break;
                        }
                    return type + " " + (*params)[1] + "() {\n";
                }
                default:
                    Notifier::notifyError(ERROR_TYPE::SYNTAX_ERROR);
                    std::cout << "[ERROR]: function had " << params->size() << " defining"<< std::endl;
                    break;
            }
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

        auto duration = duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time);
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