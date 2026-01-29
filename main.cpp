#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <list>

class Compiler {
    std::list<std::string> cppLibraryUsed;
    using ProcessingFunc = std::string(*)(std::string);
    struct Keyword {
        ProcessingFunc processing;
        std::string name;
        Keyword(std::string name, ProcessingFunc processing)
            : name(std::move(name)), processing(processing) {}
    };
    class CodeConvertionClass {
    public:
        static std::string processIf(std::string content) {
            std::string intermediateCode = "";
            return "";
        }
        static std::string processElse(std::string content) {
            return "";
        }
        static std::string processFor(std::string content) {
            return "";
        }
    };
    int activate(std::string file){
        std::vector<Keyword> keywords;
        keywords.emplace_back("if", &CodeConvertionClass::processIf);
        keywords.emplace_back("else", &CodeConvertionClass::processElse);
        keywords.emplace_back("for", &CodeConvertionClass::processFor);
        std::ifstream in(file);
        if (!in){
            std::cerr << "failed to open specified path";
            return 1;
        }
        char ch;
        while (in.get(ch)) {

        }
        in.close();
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

bool isFileValid(const std::string& filePath) {
    if (filePath.length() < 8) 
        return false;
    return filePath.rfind(".pandac") != std::string::npos
    && (filePath.find_first_of(" ") == std::string::npos)
    && (filePath.find('.') == filePath.rfind('.'));
}

int main(int argc, char** argv) {
    std::string filePath = findSpecifier(argc, argv, "-f=");
    if(!isFileValid(filePath)){
        //red
        std::cerr << "\033[1;31m";
        std::cerr << "[ERR COMPILATION]: invalid file specified" << std::endl;
        std::cerr << "\033[0m";
        return 1;
    }
    //yellow
    std::cout << "\033[1;33m";
    std::cout << "[COMPILATION]: "; 
    std::cout << "\033[0m";
    std::cout << "Compiling file: " << filePath << std::endl;
}