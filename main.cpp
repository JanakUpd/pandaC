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
        std::string name;
        Keyword(std::string name, ProcessingFunc processing)
            : name(std::move(name)), processing(processing) {}
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
    public:
        static std::set<std::string> cppLibrariesUsed;
        static std::string processIf(std::string& content) {
            auto param = getParam(content);

            return "";
        }
        static std::string processElse(std::string& content) {
            return "";
        }
        static std::string processFor(std::string& content) {
            return "";
        }
        static std::string processPrint(std::string& content) {
            cppLibrariesUsed.emplace("iostream");
            auto params = getParams(content);
            for (auto& item : params) {
                std::cout << "PRINT PARAM: " << item << std::endl;
            }
            std::cout << "TOTAL PARAMS: " << params.size() << std::endl;
            switch (params.size()) {
                case 1:
                    return "\tstd::cout << " + params[0] + " << std::endl;\n";
                case 2:
                    std::cout << "PARAMS 2: " << params[0] << " | " << params[1] << std::endl;
                    return "\tstd::cout << " + params[0] + " << " + params[1] + ";\n";
                default:
                    return "";
            }
        }
    };
    bool isFileValid(const std::string& filePath) {
        if (filePath.length() < 8) 
            return false;
        return filePath.rfind(".pandac") != std::string::npos
        && (filePath.find_first_of(" ") == std::string::npos)
        && (filePath.find('.') == filePath.rfind('.'));
    }
public:
    int activate(std::string file){
        if(!isFileValid(file)){
            Notifier::notifyError(ERROR_TYPE::FILE_NOT_FOUND);
            return 1;
        }
        Notifier::notifyInfo("Compiling file: " + file);
        std::vector<Keyword> keywords;
        // keywords.emplace_back("if", &CodeConvertionClass::processIf);
        // keywords.emplace_back("else", &CodeConvertionClass::processElse);
        // keywords.emplace_back("for", &CodeConvertionClass::processFor);
        keywords.emplace_back("print", &CodeConvertionClass::processPrint);
        std::ifstream in(file);
        if (!in){
            Notifier::notifyError(ERROR_TYPE::FILE_NOT_FOUND);
            return 1;
        }
        std::string s;
        std::string mainCode = "";
        while (getline(in, s)) {
            for (const auto& keyword : keywords) {
                if (s.find(keyword.name, 0) == 0) {
                    mainCode += keyword.processing(s);
                }
            }
        }

        if (mainCode.find("int main()") == std::string::npos)
            mainCode = "int main() {\n" + mainCode + "\treturn 0;\n}";

        std::ofstream out("pandaC.cpp");

        for (auto& item : CodeConvertionClass::cppLibrariesUsed)
            out << "#include <" << item << ">\n";
        out << mainCode;
        out.close();

        std::cout << std::endl;
        in.close();

        int compileResult = std::system("g++ pandaC.cpp -o pandaC_executable");
        if (compileResult != 0) {
            Notifier::notifyError(ERROR_TYPE::UNKNOWN_ERROR);
            return 1;
        }

        Notifier::notifyInfo("Compilation finished successfully.");
        return 0;
    }
};

std::string findSpecifier(int argc, char** argv, const std::string& specifier) {
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
    std::string filePath = findSpecifier(argc, argv, "-f=");
    Compiler compiler;
    compiler.activate(filePath);
}