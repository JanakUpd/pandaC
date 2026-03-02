#include "../../include/notifier/notifier.h"
#include <iostream>


void Notifier::notifyError(ERROR_TYPE errorType) {
    std::cerr << "\033[1;31m";
    std::cerr << "[ERR COMPILATION]: ";
    switch (errorType) {
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
void Notifier::notifyInfo(const std::string &info) {
    std::cout << "\033[1;34m";
    std::cout << "[INFO]: " << info << std::endl;
    std::cout << "\033[0m";
}
