#include <iostream>
//START OF BLOCK: pandaC

#include <iostream>
#include <string>
#include <sstream>
#include <concepts>
#include <type_traits>

// Helper to convert values to string
template <typename T>
std::string to_str(const T& val) {
    if constexpr (std::is_convertible_v<T, std::string>) {
        return std::string(val);
    } else {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }
}

// Wrapper struct
template <typename T>
struct Var {
    T value;

    // 1. Default Constructor
    Var() : value(T{}) {}

    // 2. Direct Constructor (e.g., Var<int>(5))
    Var(const T& v) : value(v) {}

    // 3. Converting Constructor (The Fix)
    // Allows Var<std::string> s = "literal"; by accepting const char* directly
    template <typename U>
    requires (!std::is_same_v<std::decay_t<U>, T> && std::convertible_to<U, T>)
    Var(const U& v) : value(v) {}

    // Implicit conversion to underlying type
    operator T&() { return value; }
    operator const T&() const { return value; }

    // Assignment
    Var& operator=(const T& v) { value = v; return *this; }
};

// Stream support
template <typename T>
std::ostream& operator<<(std::ostream& os, const Var<T>& v) {
    return os << v.value;
}

// --- Operators returning Var<std::string> to preserve type system in chains ---

// 1. Var + const char*
template <typename T>
Var<std::string> operator+(const Var<T>& lhs, const char* rhs) {
    return Var<std::string>(to_str(lhs.value) + std::string(rhs));
}

// 2. const char* + Var
template <typename T>
Var<std::string> operator+(const char* lhs, const Var<T>& rhs) {
    return Var<std::string>(std::string(lhs) + to_str(rhs.value));
}

// 3. Var + std::string
template <typename T>
Var<std::string> operator+(const Var<T>& lhs, const std::string& rhs) {
    return Var<std::string>(to_str(lhs.value) + rhs);
}

// 4. std::string + Var
template <typename T>
Var<std::string> operator+(const std::string& lhs, const Var<T>& rhs) {
    return Var<std::string>(lhs + to_str(rhs.value));
}

// 5. Var + Var (Mixed Arithmetic or String concat)
template <typename T, typename U>
auto operator+(const Var<T>& lhs, const Var<U>& rhs) {
    if constexpr (std::is_same_v<T, std::string> || std::is_same_v<U, std::string>) {
        return Var<std::string>(to_str(lhs.value) + to_str(rhs.value));
    } else {
        return Var<decltype(lhs.value + rhs.value)>(lhs.value + rhs.value);
    }
}

// 6. Var + Primitive (e.g., Var<string> + int)
template <typename T, typename U>
requires (!std::is_same_v<U, const char*> && !std::is_convertible_v<U, std::string>)
auto operator+(const Var<T>& lhs, U rhs) {
    if constexpr (std::is_same_v<T, std::string>) {
        return Var<std::string>(lhs.value + to_str(rhs));
    } else {
        return Var<decltype(lhs.value + rhs)>(lhs.value + rhs);
    }
}

// 7. Primitive + Var
template <typename T, typename U>
requires (!std::is_same_v<U, const char*> && !std::is_convertible_v<U, std::string>)
auto operator+(U lhs, const Var<T>& rhs) {
    if constexpr (std::is_same_v<T, std::string>) {
        return Var<std::string>(to_str(lhs) + rhs.value);
    } else {
        return Var<decltype(lhs + rhs.value)>(lhs + rhs.value);
    }
}
//END OF BLOCK: pandaC

int main() {
    std::cout<<"Greetings to the world of PandaC!"<<'\n';
}
