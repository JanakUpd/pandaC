#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>

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
    using ProcessingFunc = std::string(*)(std::vector<std::string>*);
    enum class Parameters {
        Bracketing,
        Spacing,
        None
    };
    struct Keyword {
        ProcessingFunc processing;
        Parameters params;
        std::string name;
        Keyword(std::string name, ProcessingFunc processing, Parameters params = Parameters::None)
            : name(std::move(name)), processing(processing), params(params) {}
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
        // static std::string findCurrentBlock(std::string& content) {
        //
        //     return "";
        // }
        // static std::vector<std::string> getParams(std::string& content) {
        //     return split(content.substr(content.find('(') + 1, content.rfind(')') - content.find('(') - 1), ',');
        // }
        // static std::string getParam(std::string& content) {
        //     return content.substr(content.find('(') + 1, content.find(')') - content.find('(') - 1);
        // }
        // static size_t countIndentation(const std::string& line) {
        //     size_t count = 0;
        //     for (auto it = line.begin(); it != line.end(); ++it) {
        //         if (*it == ' ')
        //             ++count;
        //         else if (*it == '\t')
        //             count += 4;
        //         else
        //             return count;
        //     }
        //     return count;
        // }
        // static std::string currIndentation(const std::string& line) {
        //     std::string res;
        //     for (auto it = line.begin(); it != line.end(); ++it) {
        //         if (*it == ' ')
        //             res += ' ';
        //         else if (*it == '\t')
        //             res += "    ";
        //         else
        //             return res;
        //     }
        //     return res;
        // }
        static std::string createIndentation(size_t ind) {
            std::string res;
            for (size_t i = 0; i < ind; ++i)
                res += ' ';
            return res;
        }
        static const Keyword& containsKeyword(std::string& s, const std::vector<Keyword>& keywords) {
            for (auto& item : keywords)
                if (s.size() >= item.name.size() && s.substr(s.size()-item.name.size(), item.name.size()) == item.name)
                    return item;
            return keywords.back();
        }
    public:
        static std::string convert(std::ifstream& in, const std::vector<Keyword>& keywords, const size_t currIndent = 0) {
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
                    }
                    else if (stringFlag) {
                        cnstString += c;
                    }
                    else if (c != '\n' ) {
                        line += c;
                        auto& keyword = containsKeyword(line, keywords);
                        if (keyword.name != "") {
                            switch (keyword.params) {
                                case Parameters::None:
                                    //std::cout << "[line]: " << line << " [content]: " << currContent << " ";
                                    currContent += keyword.processing(nullptr);
                                    //std::cout << currContent << std::endl;
                                    break;
                                case Parameters::Spacing: {
                                    std::vector<std::string> params;
                                    std::string buffer = "";
                                    while (in.get(c)) {
                                        if (c == '\n') {
                                            // prevIndCount = indCount;
                                            // indCount = 0;
                                            // indFlag = true;
                                            params.push_back(buffer);
                                            currContent += keyword.processing(&params);
                                            currContent = createIndentation(indCount) + currContent;
                                            //std::cout << currContent << " ";
                                            if (indCount < prevIndCount) {
                                                std::string buffer = "";
                                                for (size_t i = indCount; i < prevIndCount; i+=4)
                                                    buffer += createIndentation(i - indCount) + "}\n";
                                                currContent = createIndentation(indCount) + buffer + currContent;
                                            }
                                            //std::cout << currContent << std::endl;
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
                                            currContent += keyword.processing(&params);
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
                                    //std::cout << "[][][" << currContent << "][][]" << std::endl;
                                    // for (auto& item : params) {
                                    //     std::cout << "[0][" << item << "][0]" << std::endl;
                                    // }
                                    //currContent += keyword.processing(line, &params);
                                    //std::cout << "[][][][" << currContent << "][][][]" << std::endl;
                                }
                                    break;
                                // case Parameters::Bracketing: {
                                //     std::vector<std::string> params;
                                //     std::string buffer = "";
                                //     bool isReadingParams = false;
                                //     while (in.get(c)) {
                                //         if (c == ')') {
                                //             params.push_back(buffer);
                                //             break;
                                //         }
                                //         if (isReadingParams) {
                                //             if (c != ',')
                                //                 buffer += c;
                                //             else {
                                //                 params.push_back(buffer);
                                //                 buffer = "";
                                //             }
                                //         }
                                //         if (c == '(') isReadingParams = true;
                                //         prevC = c;
                                //     }
                                //     currContent += keyword.processing(line, &params);
                                //     }
                                //     break;
                                case Parameters::Bracketing: {
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
                                        else if (started) buffer += c;
                                        prevC = c;
                                    }
                                    currContent += keyword.processing(&params);
                                }
                                    break;

                            }
                            lineConsumed = true;
                        }
                    }
                    else {
                        currContent = createIndentation(indCount) + currContent;
                        //std::cout << currContent << " ";
                        if (indCount < prevIndCount) {
                            std::string buffer = "";
                            for (size_t i = indCount; i < prevIndCount; i+=4)
                                buffer += createIndentation(i - indCount) + "}\n";
                            currContent = createIndentation(indCount) + buffer + currContent;
                        }
                        //std::cout << currContent << std::endl;
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
                    auto& keyword = containsKeyword(line, keywords);
                    if (keyword.name != "") {
                        if (keyword.params == Parameters::None) {
                            currContent += keyword.processing(nullptr);
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
            // std::string s;
            // std::string mainCode = "";
            // auto pos = in.tellg();
            // while (getline(in, s)) {
            //     size_t indentation = countIndentation(s);
            //     if (indentation < currIndent) {
            //         in.seekg(pos);
            //         return mainCode;
            //     }
            //     for (const auto& keyword : keywords) {
            //         if (s.find(keyword.name, 0) != std::string::npos) {
            //             mainCode += keyword.processing(s);
            //             if (keyword.hasBlock)
            //                 mainCode += convert(in, keywords, indentation+4) + currIndentation(s) + "}\n";
            //             break;
            //         }
            //     }
            //     pos = in.tellg();
            // }
            //
            // return mainCode;
        }
        static std::set<std::string> cppLibrariesUsed;
        static std::string processDef(std::vector<std::string>* params) {
            if (params->back().find("main") !=  std::string::npos)
                return "int main() {\n";
            switch (params->size()) {
                case 1:
                    return "void " + (*params)[0] + "() {\n";
                case 2:
                    return (*params)[0] + " " + (*params)[1] + "() {\n";
                default:
                    std::cout << "[ERROR]: function had " << params->size() << " parameters"<< std::endl;
            }
            // auto args = getParams(content);
            // //check if def int func() {} or def func() {} - has returning type or not
            // std::string returnType, funcName;
            // if (content.find(" ", content.find("def ")+4) == std::string::npos || content.find("(", content.find("def ")+4) > content.find(" ", content.find("def ")+4)) {
            //     returnType = "void";
            //     funcName = content.substr(content.find("def ")+4, content.find("(", content.find("def ")+4) - content.find("def ")+4);
            // }
            // else {
            //     returnType = content.substr(content.find("def ")+4, content.find(" ", content.find("def ")+4) - content.find("def "));
            //     funcName = content.substr(content.find(" ", content.find("def ")+4)+1, content.find("(", content.find(" ", content.find("def ")+4)+1));
            //     //funcName = content.substr(content.find(" ", content.find("def ")+4)+1, content.find("(", content.find("def ")+4) - content.find(" ", content.find("def ")+4)-1);
            // }
            // return returnType + " " + funcName + "() {\n";
        }
        static std::string processReturn(std::vector<std::string>* params) {
            switch (params->size()) {
                case 0:
                    return "return;\n";
                case 1:
                    return "return " + (*params)[0] + ";\n";
            }
        }
        static std::string processIf(std::vector<std::string>* params) {
            return "if (" + (*params)[0] + ") {\n";
        }
        static std::string processElse(std::vector<std::string>* params) {
            return "else {\n";
        }
        static std::string processFor(std::vector<std::string>* params) {
            return "";
        };
        static std::string processPrint(std::vector<std::string>* params) {
            cppLibrariesUsed.emplace("iostream");
            switch ((*params).size()) {
                case 1:
                    return "std::cout << " + (*params)[0] + " << std::endl;\n";
                case 2:
                    return "std::cout << " + (*params)[0] + " << " + (*params)[1] + ";\n";
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
    int run(std::string file) {
        if(!isFileValid(file)) {
            Notifier::notifyError(ERROR_TYPE::FILE_NOT_FOUND);
            return 1;
        }
        Notifier::notifyInfo("Compiling file: " + file);
        std::vector<Keyword> keywords;
        keywords.emplace_back("if", &CodeConvertionClass::processIf, Parameters::Bracketing);
        keywords.emplace_back("else", &CodeConvertionClass::processElse);
        keywords.emplace_back("return", &CodeConvertionClass::processReturn, Parameters::Spacing);
        // keywords.emplace_back("for", &CodeConvertionClass::processFor);
        keywords.emplace_back("print", &CodeConvertionClass::processPrint, Parameters::Bracketing);
        keywords.emplace_back("def", &CodeConvertionClass::processDef, Parameters::Spacing);
        keywords.emplace_back("", nullptr);

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
        int compileResult = std::system("g++ pandaC.cpp -o pandaC_executable");
        if (compileResult != 0) {
            Notifier::notifyError(ERROR_TYPE::UNKNOWN_ERROR);
            return 1;
        }

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
int main(int argc, char** argv) {
    Compiler compiler;
    std::string filePath;
    if (argc != 1)
        filePath = findSpecifier(argc, argv, "-f=");
    else
        filePath = "main.pandac";
    compiler.run(filePath);
}