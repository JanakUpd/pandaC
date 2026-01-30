#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <list>
#include <set>
#include <cstdlib>

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
    using ProcessingFunc = std::string(*)(std::string&);
    struct Keyword {
        ProcessingFunc processing;
        bool hasBlock = false;
        std::string name;
        Keyword(std::string name, ProcessingFunc processing, bool hasBlock = false)
            : name(std::move(name)), processing(processing), hasBlock(hasBlock) {}
    };
    class CodeConvertionClass {
    private:
        static size_t count(std::string& content, char symb) {
            size_t res = 0;
            for (size_t i = 0; i < content.length(); ++i)
                if (content[i] == symb)
                    ++res;
            return res;
        }
        static std::vector<std::string> split(std::string content, char symb) {
            std::vector<std::string> res;
            res.reserve(count(content, symb) + 1);
            for (auto it = content.begin(); it != content.end(); ++it) {
                std::string temp;
                bool isString = false;
                while (it != content.end() && (*it != symb || isString)) {
                    if (*it == '"' or *it == '\'')
                        isString = !isString;
                    temp += *it;
                    ++it;
                }
                res.push_back(temp);
                if (it == content.end())
                    break;
            }
            return res;
        }
        static std::string findCurrentBlock(std::string& content) {

            return "";
        }
        static std::vector<std::string> getParams(std::string& content) {
            return split(content.substr(content.find('(') + 1, content.find(')') - content.find('(') - 1), ',');
        }
        static std::string getParam(std::string& content) {
            return content.substr(content.find('(') + 1, content.find(')') - content.find('(') - 1);
        }
        static size_t countIndentation(const std::string& line) {
            size_t count = 0;
            for (auto it = line.begin(); it != line.end(); ++it) {
                if (*it == ' ')
                    ++count;
                else if (*it == '\t')
                    count += 4;
                else
                    return count;
            }
            return count;
        }
        static std::string currIndentation(const std::string& line) {
            std::string res;
            for (auto it = line.begin(); it != line.end(); ++it) {
                if (*it == ' ')
                    res += ' ';
                else if (*it == '\t')
                    res += "    ";
                else
                    return res;
            }
            return res;
        }
    public:
        static std::string convert(std::ifstream& in, const std::vector<Keyword>& keywords, const size_t currIndent = 0)
        {
            std::string s;
            std::string mainCode = "";
            auto pos = in.tellg();
            while (getline(in, s)) {
                size_t indentation = countIndentation(s);
                if (indentation <= currIndent && currIndent != 0) {
                    in.seekg(pos);
                    return mainCode;
                }
                for (const auto& keyword : keywords) {
                    if (s.find(keyword.name, 0) != std::string::npos) {
                        mainCode += keyword.processing(s);
                        if (keyword.hasBlock)
                            mainCode += convert(in, keywords, indentation) + currIndentation(s) + "}\n";
                        break;
                    }
                }
                pos = in.tellg();
            }

            return mainCode;
        }
        static std::set<std::string> cppLibrariesUsed;
        static std::string processDef(std::string& content) {
            if (content.substr(content.find("def ")+4).find("main") != std::string::npos)
                return "int main() {\n";
            return "";
            //TODO: add possibility to assign self-writen funcs
        }
        static std::string processIf(std::string& content) {
            return content.substr(0, content.rfind(':')) + " {\n";
        }
        static std::string processElse(std::string& content) {
            return "";
        }
        static std::string processFor(std::string& content) {
            return "";
        };
        static std::string processPrint(std::string& content) {
            cppLibrariesUsed.emplace("iostream");
            auto params = getParams(content);
            switch (params.size()) {
                case 1:
                    return currIndentation(content) + "std::cout << " + params[0] + " << std::endl;\n";
                case 2:
                    std::cout << "PARAMS 2: " << params[0] << " | " << params[1] << std::endl;
                    return currIndentation(content) + "std::cout << " + params[0] + " << " + params[1] + ";\n";
                default:
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
    int activate(std::string file) {
        if(!isFileValid(file)) {
            Notifier::notifyError(ERROR_TYPE::FILE_NOT_FOUND);
            return 1;
        }
        Notifier::notifyInfo("Compiling file: " + file);
        std::vector<Keyword> keywords;
        keywords.emplace_back("if", &CodeConvertionClass::processIf, true);
        // keywords.emplace_back("else", &CodeConvertionClass::processElse);
        // keywords.emplace_back("for", &CodeConvertionClass::processFor);
        keywords.emplace_back("print", &CodeConvertionClass::processPrint);
        keywords.emplace_back("def", &CodeConvertionClass::processDef, true);

        std::ifstream in(file);
        if (!in) {
            Notifier::notifyError(ERROR_TYPE::FILE_NOT_FOUND);
            return 1;
        }
        std::string mainCode = CodeConvertionClass::convert(in, keywords);


        std::ofstream out("pandaC.cpp");
        for (auto& item : CodeConvertionClass::cppLibrariesUsed)
            out << "#include <" << item << ">\n";
        out << mainCode;
        out.close();
        in.close();
        /*int compileResult = std::system("g++ pandaC.cpp -o pandaC_executable");
        if (compileResult != 0) {
            Notifier::notifyError(ERROR_TYPE::UNKNOWN_ERROR);
            return 1;
        }*/

        if (mainCode.find("def main():") == std::string::npos)
            mainCode = "int main() {\n" + mainCode + "\treturn 0;\n};";

        Notifier::notifyInfo("Compilation finished successfully.");
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
// int main(int argc, char** argv) {
//     std::string filePath = findSpecifier(argc, argv, "-f=");
//     Compiler compiler;
//     compiler.activate(filePath);
// }
int main()
{
    Compiler compiler;
    compiler.activate("main.pandac");
}