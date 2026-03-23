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
    while (getline(in, line))
        ss << (line.find('#') == std::string::npos ? line : line.substr(0, line.find('#'))) << std::endl;
    in.close();
    return ss;
}