//
// Created by janak on 28.02.2026.
//

#ifndef PANDAC_CODECONVERSION_H
#define PANDAC_CODECONVERSION_H

#include <vector>
#include <string>
#include <set>
#include <fstream>
#include "compiler.h"



class CodeConvertion {
public:
    static std::vector<std::string> parseArguments(const std::string& argsStr, size_t paramFlags);
    static std::pair<size_t, std::string> parseLine(const std::string& line);
    static std::string adjustBraces(size_t currentIndent, size_t newIndent);
    static std::string createIndentation(size_t ind);
    static const Compiler::Keyword* findKeyword(const std::string& line, const std::vector<Compiler::Keyword>& keywords);
    static const Compiler::TypeBinder& findTypeBinder(std::string& s, const std::vector<Compiler::TypeBinder>& typeBinders);
    static std::string convertTypes(std::string command, const std::vector<Compiler::TypeBinder> &typeBinders);

public:
    static std::string convert(std::ifstream& in, const std::vector<Compiler::Keyword>& keywords, std::vector<Compiler::TypeBinder>& typeBinders);
    static std::set<std::string> cppLibrariesUsed;
    static std::set<std::string> pandaCLibrariesUsed;
    static std::string translateArgs(const std::string& rawArgs, const std::vector<Compiler::TypeBinder>* typeBinders);

    static int countArgs(const std::string& str);
    static std::string selectMap(std::vector<std::string>& params, const std::vector<std::string>& maps);
    static std::string convertCommand(std::vector<std::string>& args, const std::vector<std::string>& maps);
    static std::string processDef(std::vector<std::string>* params, std::vector<Compiler::TypeBinder>* typeBinders);
    static std::string processUsing(std::vector<std::string>* params, std::vector<Compiler::TypeBinder>* typeBinders);
    static std::string processReturn(std::vector<std::string>* params,  std::vector<Compiler::TypeBinder>* typeBinders);
    static std::string processIf(std::vector<std::string>* params,  std::vector<Compiler::TypeBinder>* typeBinders);
    static std::string processElse(std::vector<std::string>* params,  std::vector<Compiler::TypeBinder>* typeBinders);
    static std::string processWhile(std::vector<std::string> *params, std::vector<Compiler::TypeBinder> *typeBinders);
    static std::string processFor(std::vector<std::string> *paras, std::vector<Compiler::TypeBinder> *typeBinders);
};


#endif //PANDAC_CODECONVERSION_H