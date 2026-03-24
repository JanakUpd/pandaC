#include "preprocessor.h"
#include <string>
#include <sstream>
#include <fstream>

#include "notifier.h"
bool isFileValid(const std::string &filePath) {
    if (filePath.length() < 4)
        return false;
    return filePath.rfind(".pc") != std::string::npos
           && (filePath.find('.') == filePath.rfind('.'));
}
std::stringstream Preprocessor::run(const std::string &filePath, const bool &log) {
    if (!isFileValid(filePath)) {
        if (log) Notifier::notifyError(ERROR_TYPE::FILE_NOT_FOUND);
        throw std::runtime_error("runtime error");
    }
    std::stringstream ss;
    std::ifstream in(filePath);
    if (!in) {
        if (log) Notifier::notifyError(ERROR_TYPE::FILE_NOT_FOUND);
        throw std::runtime_error("runtime error");
    }
    std::string line;
    bool commented = false;
    while (getline(in, line)) {
        if (commented)
            continue;
        if (line.find("/*") != std::string::npos) {
            commented = true;
            continue;
        }
        if (line.find("*/") != std::string::npos) {
            commented = false;
            line = line.substr(line.find("*/"));
        }
        std::string processedLine = line.find('#') == std::string::npos ? line : line.substr(0, line.find('#'));
        bool isEmpty = true;
        for (char c : processedLine) {
            if (!std::isspace(c)) {
                isEmpty = false;
                break;
            }
        }

        if (!isEmpty)
            ss << processedLine << std::endl;
    }
    in.close();
    return ss;
}