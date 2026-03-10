#include <type_traits>
#include <iostream>
#include <string>
#include <sstream>

template <typename T>
std::string str(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}


void print(const std::string& out, const char& end = '\n') {
    std::cout << out << end;
}

#define DEFINE_CASTER(NAME, TYPE, PARSER) \
    template <typename T> \
    typename std::enable_if<std::is_convertible<T, TYPE>::value, TYPE>::type \
    NAME(T value) { \
        return static_cast<TYPE>(value); \
    } \
    TYPE NAME(const std::string& value) { \
        return static_cast<TYPE>(PARSER(value)); \
    } \
    TYPE NAME(const char* value) { \
        return NAME(std::string(value)); \
    }

DEFINE_CASTER(int16,  int16_t,  std::stoi)
DEFINE_CASTER(int32,  int32_t,  std::stoi)
DEFINE_CASTER(int64,  int64_t,  std::stoll)
DEFINE_CASTER(uint8,  uint8_t,  std::stoul)
DEFINE_CASTER(uint16, uint16_t, std::stoul)
DEFINE_CASTER(uint32, uint32_t, std::stoul)
DEFINE_CASTER(uint64, uint64_t, std::stoull)
DEFINE_CASTER(fl1, float,   std::stof)
DEFINE_CASTER(fl2, double,  std::stod)
#undef DEFINE_CASTER
