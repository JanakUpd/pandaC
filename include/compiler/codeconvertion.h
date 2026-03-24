#ifndef PANDAC_CODECONVERTION_H
#define PANDAC_CODECONVERTION_H

#include <string>
#include <vector>
#include <set>
#include "compiler.h"

class CodeConvertion {
public:
    static std::set<std::string> cppLibrariesUsed;
    static std::set<std::string> pandaCLibrariesUsed;

    static std::string convert(std::istream& in,
                               const std::vector<Compiler::Keyword>& keywords,
                               std::vector<Compiler::TypeBinder>& typeBinders);
};


#endif //PANDAC_CODECONVERTION_H