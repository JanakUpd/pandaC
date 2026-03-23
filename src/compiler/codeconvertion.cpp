#include "codeconvertion.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>

std::set<std::string> CodeConvertion::cppLibrariesUsed;
std::set<std::string> CodeConvertion::pandaCLibrariesUsed;

int CodeConvertion::parseIndex(const std::string& key) {
    try {
        if (isdigit(key[0])) return std::stoi(key);
    }
    catch (...) {
        return -1;
    }
    return -1;
}

std::string CodeConvertion::extractUntilDelimiter(const std::string& line, size_t& l, char delimiter) {
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

    return line.substr(startL, l - startL);
}

void CodeConvertion::storeCapturedParam(std::vector<std::string>& extractedParams, int index, const std::string& captured) {
    if (index >= 0) {
        if (extractedParams.size() <= index)
            extractedParams.resize(index + 1);
        extractedParams[index] = captured;
    }
}



// Matches a line against a pattern and extracts parameters into `extractedParams`.
// Returns true if the match is successful, false otherwise.
bool CodeConvertion::matchPattern(const std::string &line, const std::string &pattern, std::vector<std::string> &extractedParams) {
    size_t l = 0;
    size_t p = 0;
    extractedParams.clear();

    while (p < pattern.length()) {
        if (pattern[p] == '[') {
            size_t close = pattern.find(']', p);
            if (close == std::string::npos) return false;

            std::string key = pattern.substr(p + 1, close - p - 1);
            int index = parseIndex(key);
            p = close + 1;

            char delimiter = (p < pattern.length()) ? pattern[p] : '\0';
            std::string captured = extractUntilDelimiter(line, l, delimiter);
            if (index >= 0)
                storeCapturedParam(extractedParams, index, captured);
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

    std::string buffer;
    size_t depthBrackets = 0;
    for (char c: argsStr) {
        if (c == ',' && depthBrackets != 0) {
            params.push_back(buffer);
            buffer.clear();
        }
        else {
            if (c == '(') ++depthBrackets;
            if (c == ')') --depthBrackets;
            buffer += c;
        }
    }
    if (!buffer.empty()) params.push_back(buffer);
    return params;
}

std::pair<size_t, std::string> CodeConvertion::parseLine(const std::string &line) {
    size_t indentCount = 0;
    size_t i;
    for (i = 0; i < line.size(); ++i)
        if (line[i] == ' ')
            indentCount += 1;
        else if (line[i] == '\t')
            indentCount += 4;
        else
            break;
    std::string trimmed = line.substr(i);
    while (!trimmed.empty() && std::isspace(trimmed.back())) trimmed.pop_back();
    return {indentCount, trimmed};
}

std::string CodeConvertion::adjustBraces(size_t currentIndent, size_t newIndent) {
    std::string result{};
    if (currentIndent > newIndent)
        for (size_t i = currentIndent; i > newIndent; i -= 4)
            result += CodeConvertion::createIndentation(i - 4) + "}\n";
    else
        for (size_t i = currentIndent; i < newIndent; i += 4)
            result += CodeConvertion::createIndentation(i) + "{\n";
    return result;
}

std::string CodeConvertion::createIndentation(size_t ind) {
    std::string res;
    for (size_t i = 0; i < ind; ++i)
        res += ' ';
    return res;
}

const Compiler::Keyword* CodeConvertion::findKeyword(const std::string &line,
                                                     const std::vector<Compiler::Keyword> &keywords) {
    for (const auto &item: keywords)
        if (!item.name.empty() && line.starts_with(item.name))
            return &item;
    return nullptr;
}

const Compiler::TypeBinder &CodeConvertion::findTypeBinder(const std::string& s,
                                                           const std::vector<Compiler::TypeBinder> &typeBinders) {
    for (auto &item: typeBinders)
        if (s.size() >= item.pandacName.size() && s.substr(s.size() - item.pandacName.size(), item.pandacName.size()) ==
            item.pandacName)
            return item;
    return typeBinders.back();
}

std::string CodeConvertion::convertTypes(std::string_view command, const std::vector<Compiler::TypeBinder> &typeBinders) {
    std::string result(command);
    for (const auto &item: typeBinders) {
        if (item.pandacName.empty()) continue;
        size_t pos = 0;
        while ((pos = command.find(item.pandacName, pos)) != std::string::npos) {
            bool leftOk = (pos == 0 || (!std::isalnum(command[pos - 1]) && command[pos - 1] != '_'));
            bool rightOk = (pos + item.pandacName.size() >= command.size() ||
                            (!std::isalnum(command[pos + item.pandacName.size()]) && command[
                                 pos + item.pandacName.size()] != '_'));

            if (leftOk && rightOk) {
                result.replace(pos, item.pandacName.size(), item.cppName);
                pos += item.cppName.size();
            }
            else {
                pos += item.pandacName.size();
            }
        }
    }
    return result;
}

std::string CodeConvertion::convert(std::istream& in, const std::vector<Compiler::Keyword> &keywords, std::vector<Compiler::TypeBinder> &typeBinders) {
    std::string finalCppCode;
    std::string line;
    size_t currentIndent = 0;

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto [lineIndent, command] = parseLine(line);
        if (command.empty()) continue;

        if (lineIndent != currentIndent)
            finalCppCode += adjustBraces(currentIndent, lineIndent);

        if (command.starts_with("def ")) {
            finalCppCode += createIndentation(lineIndent);
            finalCppCode += processDef(command, typeBinders);
        }
        else if (command.starts_with("using "))
            pandaCLibrariesUsed.emplace(command.substr(6));
        else {
            bool matchedKeyword = false;
            auto keyword = findKeyword(command, keywords);
            if (keyword != nullptr) {
                for (const auto &mapEntry: keyword->maps) {
                    std::vector<std::string> capturedArgs;

                    if (matchPattern(command, mapEntry.first, capturedArgs)) {
                        std::string result = mapEntry.second;
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
                finalCppCode += createIndentation(lineIndent) + command + ";\n";
            }
        }
        currentIndent = lineIndent;
    }
    finalCppCode += adjustBraces(currentIndent, 0);
    return finalCppCode;
}

std::string CodeConvertion::translateArgs(const std::string &rawArgs, const std::vector<Compiler::TypeBinder> *typeBinders) {
    if (rawArgs.empty()) return "";
    std::string result;
    std::string currentArg;
    std::vector<std::string> args;
    int depth = 0;
    for (char c: rawArgs) {
        if (c == ',' && depth == 0) {
            args.push_back(currentArg);
            currentArg.clear();
        }
        else {
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

size_t CodeConvertion::countArgs(const std::string &str) {
    size_t count = 0;
    size_t shift = 0;
    while (shift < str.size())
        if (str.find('[', shift) != std::string::npos) {
            shift = str.find(']', shift) + 1;
            ++count;
        } else
            break;
    return count;
}

std::string CodeConvertion::selectMap(const std::vector<std::string> &params, const std::vector<std::string> &maps) {
    size_t count = params.size();
    for (auto &item: maps)
        if (countArgs(item) == count)
            return item;
    return "";
}

std::string CodeConvertion::convertCommand(const std::vector<std::string> &params, const std::vector<std::string> &maps) {
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

std::string CodeConvertion::processDef(const std::string& line, const std::vector<Compiler::TypeBinder>& typeBinders) {
    std::vector<std::string> params;
    std::string sub = line.substr(4);
    std::string temp;
    for (char c : sub) {
        if (c == '(') break;
        if (c == ' ') {
            if (!temp.empty()) params.push_back(temp);
            temp.clear();
        }
        else temp += c;
    }
    if (!temp.empty()) params.push_back(temp);
    size_t startArgs = sub.find('(');
    size_t endArgs = sub.rfind(')');
    //Todo: make params of def be converted as well
    if (startArgs != std::string::npos && endArgs != std::string::npos)
        params.push_back(sub.substr(startArgs + 1, endArgs - startArgs - 1));

    if (params.empty()) return "";
    for (const auto &p: params)
        if (p == "main") return "int main()\n";

    std::string returnType = "void";
    std::string funcName;
    std::string rawArgs;

    if (params.size() == 1) {
        funcName = (params)[0];
    }
    else if (params.size() == 2) {
        bool hasType = false;
        for (const auto &item: typeBinders) {
            if (params[0] == item.pandacName) {
                returnType = item.cppName;
                hasType = true;
                break;
            }
        }
        if (hasType)
            funcName = params[1];
        else {
            funcName = params[0];
            rawArgs = params[1];
        }
    }
    else if (params.size() >= 3) {
        for (const auto &item: typeBinders) {
            if (params[0] == item.pandacName) {
                returnType = item.cppName;
                break;
            }
        }
        funcName = params[1];
        rawArgs = params[2];
    }
    std::string translatedArgs = translateArgs(rawArgs, &typeBinders);
    return returnType + " " + funcName + "(" + translatedArgs + ")\n";
}