
#ifndef PANDAC_NOTIFIER_H
#define PANDAC_NOTIFIER_H
#include <string>

enum class ERROR_TYPE {
    FILE_NOT_FOUND,
    SYNTAX_ERROR,
    UNKNOWN_ERROR
};

class Notifier{
public:
    static void notifyError(ERROR_TYPE errorType);
    static void notifyInfo(const std::string& info);
};


#endif //PANDAC_NOTIFIER_H