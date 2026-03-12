#include <iostream>
//START OF BLOCK: pandaC

#include <iostream>
#include <string>
#include <sstream>
#include <concepts>
#include <type_traits>

template <typename T>
std::string to_str(const T& val) {
    std::stringstream ss;
    ss << val;
    return ss.str();
}

template <typename T>
struct Var {
    T value;
    Var() : value(0) {}
    Var(T v) : value(v) {}
    operator T&() { return value; }
    operator const T&() const { return value; }
    Var& operator=(const T& v) { value = v; return *this; }
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Var<T>& v) {
    return os << v.value;
}

template <typename T>
std::string operator+(const Var<T>& lhs, const char* rhs) {
    return to_str(lhs.value) + std::string(rhs);
}

template <typename T>
std::string operator+(const char* lhs, const Var<T>& rhs) {
    return std::string(lhs) + to_str(rhs.value);
}

template <typename T>
std::string operator+(const Var<T>& lhs, const std::string& rhs) {
    return to_str(lhs.value) + rhs;
}

template <typename T>
std::string operator+(const std::string& lhs, const Var<T>& rhs) {
    return lhs + to_str(rhs.value);
}

template <typename T, typename U>
auto operator+(const Var<T>& lhs, const Var<U>& rhs) -> Var<decltype(lhs.value + rhs.value)> {
    return Var<decltype(lhs.value + rhs.value)>(lhs.value + rhs.value);
}

template <typename T, typename U>
requires std::is_arithmetic_v<U>
auto operator+(const Var<T>& lhs, U rhs) -> Var<decltype(lhs.value + rhs)> {
    return Var<decltype(lhs.value + rhs)>(lhs.value + rhs);
}

template <typename T, typename U>
requires std::is_arithmetic_v<U>
auto operator+(U lhs, const Var<T>& rhs) -> Var<decltype(lhs + rhs.value)> {
    return Var<decltype(lhs + rhs.value)>(lhs + rhs.value);
}
//END OF BLOCK: pandaC

Var<int32_t> test(Var<int32_t> a, Var<int32_t> b) {
    return a + b;
}
int main() {
    Var<int32_t> result = test(5, 10);
    std::cout<<result + "s"<<' ';
}
