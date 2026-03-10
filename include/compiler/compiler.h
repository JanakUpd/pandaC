#include <vector>
#ifndef PANDAC_COMPILER_H
#define PANDAC_COMPILER_H


class Compiler {
public:
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
        static std::vector<std::string> parseArguments(const std::string& argsStr, size_t paramFlags);
        static std::pair<size_t, std::string> parseLine(const std::string& line);
        static std::string adjustBraces(size_t currentIndent, size_t newIndent);
        static std::string createIndentation(size_t ind);
        static const Keyword* findKeyword(const std::string& line, const std::vector<Keyword>& keywords);
        static const TypeBinder& findTypeBinder(std::string& s, const std::vector<TypeBinder>& typeBinders);
    public:
        static std::string convert(std::ifstream& in, const std::vector<Keyword>& keywords, std::vector<TypeBinder>& typeBinders);
        static std::set<std::string> cppLibrariesUsed;
        static std::set<std::string> pandaCLibrariesUsed;
        static std::string translateArgs(const std::string& rawArgs, const std::vector<TypeBinder>* typeBinders);
        static std::string processDef(std::vector<std::string>* params, std::vector<TypeBinder>* typeBinders);
        static std::string processUsing(std::vector<std::string>* params, std::vector<TypeBinder>* typeBinders);
        static std::string processReturn(std::vector<std::string>* params,  std::vector<TypeBinder>* typeBinders);
        static std::string processIf(std::vector<std::string>* params, std::vector<TypeBinder>* typeBinders);
        static std::string processElse(std::vector<std::string>* params, std::vector<TypeBinder>* typeBinders);
        static std::string processFor(std::vector<std::string>* params, std::vector<TypeBinder>* typeBinders);
        static std::string processPrint(std::vector<std::string>* params, std::vector<TypeBinder>* typeBinders);
    };
    int countIndentation(const std::string& line);
    bool isFileValid(const std::string& filePath);
public:
    int run(std::string file, bool execute = false, bool log = false);
};


#endif //PANDAC_COMPILER_H