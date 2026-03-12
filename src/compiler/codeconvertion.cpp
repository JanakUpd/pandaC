#include "codeconvertion.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>

std::set<std::string> CodeConvertion::cppLibrariesUsed;
std::set<std::string> CodeConvertion::pandaCLibrariesUsed;

bool matchPattern(const std::string &line, const std::string &pattern, std::vector<std::string> &extractedParams) {
    size_t l = 0;
    size_t p = 0;
    extractedParams.clear();

    while (p < pattern.length()) {
        if (pattern[p] == '[') {
            size_t close = pattern.find(']', p);
            if (close == std::string::npos) return false;

            std::string key = pattern.substr(p + 1, close - p - 1);
            int index = -1;
            try {
                if (isdigit(key[0])) index = std::stoi(key);
            } catch (...) { index = -1; }

            p = close + 1;

            char delimiter = (p < pattern.length()) ? pattern[p] : '\0';

            size_t startL = l;
            int bracketDepth = 0;
            bool inQuote = false;
            char quoteChar = 0;

            while (l < line.length()) {
                char c = line[l];
                if (inQuote) {
                    if (c == '\\') l++;
                    else if (c == quoteChar) inQuote = false;
                }
                else {
                    if (bracketDepth == 0 && (c == delimiter || (c == ',' && delimiter != ',')))
                        break;
                    if (c == '(' || c == '{' || c == '[')
                        bracketDepth++;
                    else if (c == ')' || c == '}' || c == ']') {
                        if (bracketDepth > 0)
                            bracketDepth--;
                    }
                    else if (c == '"' || c == '\'') {
                        inQuote = true;
                        quoteChar = c;
                    }
                }
                l++;
            }

            std::string captured = line.substr(startL, l - startL);

            if (delimiter != '\0')
                if (!(l < line.length() && line[l] == delimiter))
                    return false;

            if (index >= 0) {
                if (extractedParams.size() <= index) extractedParams.resize(index + 1);
                extractedParams[index] = captured;
            }
        }
        else {
            if (l >= line.length() || line[l] != pattern[p])
                return false;
            l++;
            p++;
        }
    }
    return l == line.length();
}

std::vector<std::string> CodeConvertion::parseArguments(const std::string &argsStr, size_t paramFlags) {
    std::vector<std::string> params;
    bool useSpacing = (paramFlags & static_cast<size_t>(Compiler::Parameters::Spacing)) != 0;
    bool useBracketing = (paramFlags & static_cast<size_t>(Compiler::Parameters::Bracketing)) != 0;

    std::string buffer;
    bool inBrackets = false;
    for (char c: argsStr) {
        if (c == ',' && !inBrackets) {
            params.push_back(buffer);
            buffer.clear();
        } else {
            if (c == '(') inBrackets = true;
            if (c == ')') inBrackets = false;
            buffer += c;
        }
    }
    if (!buffer.empty()) params.push_back(buffer);
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
    while (!trimmed.empty() && std::isspace(trimmed.back())) trimmed.pop_back();
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

const Compiler::Keyword *CodeConvertion::findKeyword(const std::string &line,
                                                     const std::vector<Compiler::Keyword> &keywords) {
    for (const auto &item: keywords) {
        if (!item.name.empty() && line.starts_with(item.name)) {
            return &item;
        }
    }
    return nullptr;
}

const Compiler::TypeBinder &CodeConvertion::findTypeBinder(std::string &s,
                                                           const std::vector<Compiler::TypeBinder> &typeBinders) {
    for (auto &item: typeBinders)
        if (s.size() >= item.pandacName.size() && s.substr(s.size() - item.pandacName.size(), item.pandacName.size()) ==
            item.pandacName)
            return item;
    return typeBinders.back();
}

std::string CodeConvertion::convertTypes(std::string command, const std::vector<Compiler::TypeBinder> &typeBinders) {
    for (const auto &item: typeBinders) {
        if (item.pandacName.empty()) continue;
        size_t pos = 0;
        while ((pos = command.find(item.pandacName, pos)) != std::string::npos) {
            bool leftOk = (pos == 0 || (!std::isalnum(command[pos - 1]) && command[pos - 1] != '_'));
            bool rightOk = (pos + item.pandacName.size() >= command.size() ||
                            (!std::isalnum(command[pos + item.pandacName.size()]) && command[
                                 pos + item.pandacName.size()] != '_'));

            if (leftOk && rightOk) {
                command.replace(pos, item.pandacName.size(), item.cppName);
                pos += item.cppName.size();
            } else {
                pos += item.pandacName.size();
            }
        }
    }
    return command;
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

        if (command.starts_with("def ")) {
            std::string sub = command.substr(4);
            std::vector<std::string> parts;
            std::stringstream ss(sub);
            std::string buf;
            while (getline(ss, buf, '(')) {
                parts.push_back(buf);
                break;
            }
            std::vector<std::string> defParams = parseArguments(sub, 0);
            std::vector<std::string> tokens;
            std::string temp;
            for (char c: sub) {
                if (c == '(') break;
                if (c == ' ') {
                    if (!temp.empty()) tokens.push_back(temp);
                    temp.clear();
                } else temp += c;
            }
            if (!temp.empty()) tokens.push_back(temp);
            size_t startArgs = sub.find('(');
            size_t endArgs = sub.rfind(')');
            if (startArgs != std::string::npos && endArgs != std::string::npos) {
                tokens.push_back(sub.substr(startArgs + 1, endArgs - startArgs - 1));
            }

            finalCppCode += createIndentation(lineIndent);
            finalCppCode += processDef(&tokens, &typeBinders);
        }
        else if (command.starts_with("using ")) {
            std::vector<std::string> params{command.substr(6)};
            processUsing(&params, &typeBinders);
        }
        else {
            bool matchedKeyword = false;
            auto *keyword = findKeyword(command, keywords);
            if (keyword) {
                for (const auto &mapEntry: keyword->maps) {
                    size_t sep = mapEntry.find("@@@");
                    if (sep == std::string::npos) continue;

                    std::string pattern = mapEntry.substr(0, sep);
                    std::string codeTemplate = mapEntry.substr(sep + 3);
                    std::vector<std::string> capturedArgs;

                    if (matchPattern(command, pattern, capturedArgs)) {
                        std::string result = codeTemplate;
                        for (size_t i = 0; i < capturedArgs.size(); ++i) {
                            std::string pl = "[" + std::to_string(i) + "]";
                            size_t pos = 0;
                            while ((pos = result.find(pl, pos)) != std::string::npos) {
                                result.replace(pos, pl.length(), capturedArgs[i]);
                                pos += capturedArgs[i].length();
                            }
                        }
                        finalCppCode += createIndentation(lineIndent) + result + "\n";
                        matchedKeyword = true;
                        break;
                    }
                }
            }

            if (!matchedKeyword) {
                command = convertTypes(command, typeBinders);
                if (command.starts_with("if ")) {
                    std::string cond = command.substr(3);
                    std::vector<std::string> p{cond};
                    finalCppCode += createIndentation(lineIndent) + processIf(&p, &typeBinders);
                }
                else if (command.starts_with("else")) {
                    std::vector<std::string> p;
                    finalCppCode += createIndentation(lineIndent) + processElse(&p, &typeBinders);
                }
                else if (command.starts_with("return")) {
                    std::string val = (command.length() > 6) ? command.substr(7) : "";
                    std::vector<std::string> p{val};
                    finalCppCode += createIndentation(lineIndent) + processReturn(&p, &typeBinders);
                }
                else
                    finalCppCode += createIndentation(lineIndent) + command + ";\n";
            }
        }
        currentIndent = lineIndent;
    }
    finalCppCode += adjustBraces(currentIndent, 0);
    return finalCppCode;
}

static std::set<std::string> cppLibrariesUsed;
static std::set<std::string> pandaCLibrariesUsed;

std::string CodeConvertion::translateArgs(const std::string &rawArgs,
                                          const std::vector<Compiler::TypeBinder> *typeBinders) {
    if (rawArgs.empty()) return "";
    std::string result;
    std::string currentArg;
    std::vector<std::string> args;
    int depth = 0;
    for (char c: rawArgs) {
        if (c == ',' && depth == 0) {
            args.push_back(currentArg);
            currentArg.clear();
        } else {
            if (c == '<' || c == '(') depth++;
            if (c == '>' || c == ')') depth--;
            currentArg += c;
        }
    }
    args.push_back(currentArg);

    for (size_t i = 0; i < args.size(); ++i) {
        std::string s = args[i];
        size_t first = s.find_first_not_of(" \t");
        if (first == std::string::npos) continue;
        s = s.substr(first);
        size_t last = s.find_last_not_of(" \t");
        if (last != std::string::npos) s = s.substr(0, last + 1);

        if (i > 0) result += ", ";

        size_t space = s.find(' ');
        if (space != std::string::npos) {
            std::string typePart = s.substr(0, space);
            std::string namePart = s.substr(space + 1);

            bool foundType = false;
            for (const auto &tb: *typeBinders) {
                if (tb.pandacName == typePart) {
                    result += tb.cppName + " " + namePart;
                    foundType = true;
                    break;
                }
            }
            if (!foundType) result += s;
        } else {
            result += s;
        }
    }
    return result;
}

int CodeConvertion::countArgs(const std::string &str) {
    int count = 0;
    int shift = 0;
    while (shift < str.size())
        if (str.find('[', shift) != std::string::npos) {
            shift = str.find(']', shift) + 1;
            ++count;
        } else
            break;
    return count;
}

std::string CodeConvertion::selectMap(std::vector<std::string> &params, const std::vector<std::string> &maps) {
    int count = params.size();
    for (auto &item: maps)
        if (countArgs(item) == count)
            return item;
    return "";
}

std::string CodeConvertion::convertCommand(std::vector<std::string> &params, const std::vector<std::string> &maps) {
    std::string result = selectMap(params, maps);
    if (result.empty()) return "";

    for (size_t i = 0; i < params.size(); ++i) {
        std::string placeholder = "[" + std::to_string(i) + "]";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), params[i]);
            pos += params[i].length();
        }
    }
    if (!result.empty() && result.back() != '\n') {
        result += "\n";
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
    if (params->empty()) return "return;\n";
    std::string res = "return ";
    for (size_t i = 0; i < params->size(); ++i) {
        if (i > 0) res += " ";
        res += (*params)[i];
    }
    res += ";\n";
    return res;
}
