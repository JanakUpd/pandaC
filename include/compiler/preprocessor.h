#ifndef PANDAC_PREPROCESSOR_H
#define PANDAC_PREPROCESSOR_H
#include <string>
#include <sstream>
class Preprocessor {
    public:
    static std::stringstream run(const std::string &filePath, const bool &log);
};
#endif