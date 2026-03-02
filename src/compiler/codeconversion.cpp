#include "codeconversion.h"
#include "notifier.h"
#include <iostream>

std::set<std::string> CodeConvertion::cppLibrariesUsed;
std::set<std::string> CodeConvertion::pandaCLibrariesUsed;

std::vector<std::string> CodeConvertion::parseArguments(const std::string &argsStr, size_t paramFlags) {
    std::vector<std::string> params;
    bool useSpacing = (paramFlags & static_cast<size_t>(Compiler::Parameters::Spacing)) != 0;
    bool useBracketing = (paramFlags & static_cast<size_t>(Compiler::Parameters::Bracketing)) != 0;

    std::string buffer;
    size_t depth = 0;
    bool inBrackets = false;

    for (char c: argsStr) {
        if (useBracketing && c == '(') {
            if (depth == 0) {
                if (useSpacing && !buffer.empty()) {
                    params.push_back(buffer);
                    buffer.clear();
                }
                inBrackets = true;
            } else {
                buffer += c;
            }
            ++depth;
        } else if (useBracketing && c == ')') {
            depth--;
            if (depth == 0) {
                params.push_back(buffer);
                buffer.clear();
                inBrackets = false;
            } else {
                buffer += c;
            }
        } else if (inBrackets) {
            buffer += c;
        } else if (useSpacing) {
            if (std::isspace(c) || c == ':') {
                if (!buffer.empty()) {
                    params.push_back(buffer);
                    buffer.clear();
                }
            } else {
                buffer += c;
            }
        }
    }
    if (!buffer.empty())
        params.push_back(buffer);

    return params;
}

std::pair<size_t, std::string> CodeConvertion::parseLine(const std::string &line) {
    size_t indentCount = 0;
    size_t i = 0;
    while (i < line.size()) {
        if (line[i] == ' ') {
            indentCount += 1;
            i++;
        } else if (line[i] == '\t') {
            indentCount += 4;
            i++;
        } else break;
    }
    std::string trimmed = line.substr(i);
    while (!trimmed.empty() && std::isspace(trimmed.back())) {
        trimmed.pop_back();
    }
    return {indentCount, trimmed};
}

std::string CodeConvertion::adjustBraces(size_t currentIndent, size_t newIndent) {
    std::string result = "";
    for (size_t i = currentIndent; i > newIndent; i -= 4) {
        result += CodeConvertion::createIndentation(i - 4) + "}\n";
    }
    return result;
}

std::string CodeConvertion::createIndentation(size_t ind) {
    std::string res;
    for (size_t i = 0; i < ind; ++i)
        res += ' ';
    return res;
}

const Compiler::Keyword* CodeConvertion::findKeyword(const std::string &line, const std::vector<Compiler::Keyword> &keywords) {
    for (const auto &item: keywords) {
        if (!item.name.empty() && line.starts_with(item.name)) {
            return &item;
        }
    }
    return nullptr;
}

const Compiler::TypeBinder& CodeConvertion::findTypeBinder(std::string &s, const std::vector<Compiler::TypeBinder> &typeBinders) {
    for (auto &item: typeBinders)
        if (s.size() >= item.pandacName.size() && s.substr(s.size() - item.pandacName.size(), item.pandacName.size()) ==
            item.pandacName)
            return item;
    return typeBinders.back();
}

std::string CodeConvertion::convert(std::ifstream &in, const std::vector<Compiler::Keyword> &keywords,
                           std::vector<Compiler::TypeBinder> &typeBinders) {
    std::string finalCppCode;
    std::string line;
    size_t currentIndent = 0;

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto [lineIndent, command] = parseLine(line);
        if (command.empty()) continue;
        if (lineIndent < currentIndent)
            finalCppCode += adjustBraces(currentIndent, lineIndent);

        auto *keyword = findKeyword(command, keywords);
        if (keyword) {
            std::string argsStr = command.substr(keyword->name.size());
            std::vector<std::string> params = parseArguments(argsStr, keyword->params);
            finalCppCode += createIndentation(lineIndent);
            finalCppCode += keyword->processing(&params, &typeBinders);
        } else {
            finalCppCode += createIndentation(lineIndent) + command + ";\n";
        }

        currentIndent = lineIndent;
    }
    finalCppCode += adjustBraces(currentIndent, 0);
    return finalCppCode;
}

static std::set<std::string> cppLibrariesUsed;
static std::set<std::string> pandaCLibrariesUsed;

std::string CodeConvertion::translateArgs(const std::string &rawArgs, const std::vector<Compiler::TypeBinder> *typeBinders) {
    if (rawArgs.empty()) return "";

    std::string result = "";
    std::string currentArg = "";

    std::string argsToParse = rawArgs + ",";
    bool first = true;

    for (char c: argsToParse) {
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
                    for (const auto &tb: *typeBinders) {
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

std::string CodeConvertion::processDef(std::vector<std::string> *params,
                                              std::vector<Compiler::TypeBinder> *typeBinders) {
    if (params->empty()) return "";
    for (const auto &p: *params) {
        if (p == "main") return "int main() {\n";
    }

    std::string returnType = "void";
    std::string funcName = "";
    std::string rawArgs = "";

    if (params->size() == 1) {
        funcName = (*params)[0];
    } else if (params->size() == 2) {
        bool hasType = false;
        for (const auto &item: *typeBinders) {
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
    } else if (params->size() >= 3) {
        for (const auto &item: *typeBinders) {
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

std::string CodeConvertion::processUsing(std::vector<std::string> *params,
                                                std::vector<Compiler::TypeBinder> *typeBinders) {
    if (params->size() == 1)
        pandaCLibrariesUsed.emplace((*params)[0]);
    return "";
}

std::string CodeConvertion::processReturn(std::vector<std::string> *params,
                                                 std::vector<Compiler::TypeBinder> *typeBinders) {
    switch (params->size()) {
        case 1: return "return " + (*params)[0] + ";\n";
        default: return "return;\n";
    }
}

std::string CodeConvertion::processIf(std::vector<std::string> *params,
                                             std::vector<Compiler::TypeBinder> *typeBinders) {
    return "if (" + (*params)[0] + ") {\n";
}

std::string CodeConvertion::processElse(std::vector<std::string> *params,
                                               std::vector<Compiler::TypeBinder> *typeBinders) {
    return "else {\n";
}

std::string CodeConvertion::processFor(std::vector<std::string> *params,
                                              std::vector<Compiler::TypeBinder> *typeBinders) {
    return "";
};
std::string CodeConvertion::processPrint(std::vector<std::string> *params,
                                                std::vector<Compiler::TypeBinder> *typeBinders) {
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
