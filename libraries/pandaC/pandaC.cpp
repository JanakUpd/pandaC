#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <type_traits>
#include <stdexcept>
#include <sstream>

namespace PandaC {

template <typename T>
class PandaCList {
public:
    std::vector<T> data;

    PandaCList() = default;
    PandaCList(std::initializer_list<T> init) : data(init) {}

    int64_t length() const { return static_cast<int64_t>(data.size()); }

    T& operator[](size_t idx) {
        if (idx >= data.size()) throw std::runtime_error("List index out of bounds");
        return data[idx];
    }
    const T& operator[](size_t idx) const {
        if (idx >= data.size()) throw std::runtime_error("List index out of bounds");
        return data[idx];
    }

    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
};

template <typename K, typename V>
struct PandaCKVPair {
    K key;
    V value;
};

template <typename K, typename V>
PandaCKVPair(K, V) -> PandaCKVPair<K, V>;

template <typename K, typename V>
class PandaCDict {
public:
    std::unordered_map<K, V> data;

    PandaCDict() = default;

    PandaCDict(std::initializer_list<PandaCKVPair<K, V>> init) {
        for (const auto& kv : init) {
            data[kv.key] = kv.value;
        }
    }

    int64_t length() const { return static_cast<int64_t>(data.size()); }

    V& operator[](const K& key) {
        return data[key];
    }
    const V& operator[](const K& key) const {
        auto it = data.find(key);
        if (it == data.end()) throw std::runtime_error("Key not found");
        return it->second;
    }
};
    // class Vector {
    //     std::vector<double> data;
    //     size_t dataSize;
    // public:
    //     double operator[] (const size_t& ind) {
    //         return data[ind];
    //     }
    //     Vector(PandaCList<double> data) {
    //         dataSize = data.length();
    //         this->data.reserve(dataSize);
    //         for (size_t i = 0; i < num_rows; ++i) {
    //             if (data[i].length() != num_cols) throw std::runtime_error("All rows must have the same number of columns");
    //             for (size_t j = 0; j < num_cols; ++j) {
    //                 this->data.push_back(data[i][j]);
    //             }
    //         }
    //     }
    //     size_t getLength() {
    //         return num_rows;
    //     }
    //     bool empty() const {
    //         return data.empty();
    //     }
    // };
class Matrix {
    std::vector<double> data;
    size_t num_cols;
    size_t num_rows;
public:
    double operator[] (const size_t& col, const size_t &row) {
        return data[row * num_cols + col];
    }
    // double operator[] (const size_t& ind) {
    //     Vector res = Vector(num_rows);
    //
    //     return data[ind];
    // }
    Matrix(PandaCList<PandaCList<double>> data) {
        num_rows = data.length();
        num_cols = data[0].length();
        this->data.reserve(num_rows * num_cols);
        for (size_t i = 0; i < num_rows; ++i) {
            if (data[i].length() != num_cols) throw std::runtime_error("All rows must have the same number of columns");
            for (size_t j = 0; j < num_cols; ++j) {
                this->data.push_back(data[i][j]);
            }
        }
    }
    size_t getCols(){
        return num_cols;
    }
    size_t getRows() {
        return num_rows;
    }
    bool empty() const {
        return data.empty();
    }
};


template <typename T>
std::ostream& operator<<(std::ostream& os, const PandaCList<T>& list) {
    os << "[";
    bool first = true;
    for (const auto& item : list.data) {
        if (!first) os << ", ";
        if constexpr (std::is_same_v<T, std::string>) os << "'" << item << "'";
        else os << item;
        first = false;
    }
    os << "]";
    return os;
}

template <typename K, typename V>
std::ostream& operator<<(std::ostream& os, const PandaCDict<K, V>& dict) {
    os << "{";
    bool first = true;
    for (const auto& kv : dict.data) {
        if (!first) os << ", ";

        if constexpr (std::is_same_v<K, std::string>) os << "'" << kv.first << "': ";
        else os << kv.first << ": ";

        if constexpr (std::is_same_v<V, std::string>) os << "'" << kv.second << "'";
        else os << kv.second;

        first = false;
    }
    os << "}";
    return os;
}

template <typename T>
inline PandaCList<T> make_pandac_list(std::initializer_list<T> init) {
    return PandaCList<T>(init);
}

template <typename K, typename V>
inline PandaCDict<K, V> make_pandac_dict(std::initializer_list<PandaCKVPair<K, V>> init) {
    return PandaCDict<K, V>(init);
}

inline void pandac_print() { std::cout << std::endl; }

template <typename T, typename... Args>
void pandac_print(const T& first, const Args&... args) {
    std::cout << first;
    if constexpr (sizeof...(args) > 0) {
        std::cout << " ";
        pandac_print(args...);
    } else {
        std::cout << std::endl;
    }
}

template <typename T>
int64_t pandac_len(const T& container) {
    if constexpr (std::is_same_v<T, std::string>) {
        return static_cast<int64_t>(container.length());
    } else {
        return container.length();
    }
}

inline std::string input(const std::string& prompt = "") {
    if (!prompt.empty()) std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

struct PandaRange {
    int64_t start_; int64_t end_; int64_t step_;
    PandaRange(int64_t s, int64_t e, int64_t st) : start_(s), end_(e), step_(st) {}

    struct Iterator {
        int64_t current_; int64_t step_;
        Iterator(int64_t current, int64_t step) : current_(current), step_(step) {}
        int64_t operator*() const { return current_; }
        Iterator& operator++() { current_ += step_; return *this; }
        bool operator!=(const Iterator& other) const {
            if (step_ > 0) return current_ < other.current_;
            return current_ > other.current_;
        }
    };
    Iterator begin() const { return Iterator(start_, step_); }
    Iterator end() const { return Iterator(end_, step_); }
};

inline PandaRange range(long long start, long long end, long long step = 1) { return PandaRange(start, end, step); }
inline PandaRange range(long long end) { return PandaRange(0, end, 1); }

template <typename T>
std::string pandac_str(const T& val) {
    if constexpr (std::is_same_v<T, std::string>) return val;
    else {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }
}

template <typename T>
int64_t pandac_int(const T& val) {
    if constexpr (std::is_same_v<T, std::string>) return std::stoll(val);
    else return static_cast<int64_t>(val);
}

template <typename T>
double pandac_float(const T& val) {
    if constexpr (std::is_same_v<T, std::string>) return std::stod(val);
    else return static_cast<double>(val);
}

template <typename T>
bool pandac_bool(const T& val) {
    return static_cast<bool>(val);
}

template <typename A, typename B>
inline auto pandac_mul(const A& a, const B& b) {
    if constexpr (std::is_same_v<A, std::string> && std::is_integral_v<B>) {
        std::string res; for(B i = 0; i < b; ++i) res += a; return res;
    } else if constexpr (std::is_integral_v<A> && std::is_same_v<B, std::string>) {
        std::string res; for(A i = 0; i < a; ++i) res += b; return res;
    } else {
        return a * b;
    }
}

template <typename A, typename B>
inline auto pandac_add(const A& a, const B& b) {
    if constexpr (std::is_same_v<A, std::string> || std::is_same_v<B, std::string>) {
        return pandac_str(a) + pandac_str(b);
    } else {
        return a + b;
    }
}

template <typename A, typename B> inline auto pandac_sub(const A& a, const B& b) { return a - b; }
template <typename A, typename B> inline auto pandac_div(const A& a, const B& b) { return a / b; }
template <typename A, typename B> inline auto pandac_int64_div(const A& a, const B& b) { return static_cast<int64_t>(a) / static_cast<int64_t>(b); }
template <typename A, typename B> inline auto pandac_mod(const A& a, const B& b) {
    if constexpr (std::is_integral_v<A> && std::is_integral_v<B>) return a % b;
    else return std::fmod(static_cast<double>(a), static_cast<double>(b));
}
template <typename A, typename B> inline auto pandac_pow(const A& a, const B& b) { return std::pow(static_cast<double>(a), static_cast<double>(b)); }

template <typename A, typename B> inline auto pandac_eq(const A& a, const B& b) { return a == b; }
template <typename A, typename B> inline auto pandac_neq(const A& a, const B& b) { return a != b; }
template <typename A, typename B> inline auto pandac_less(const A& a, const B& b) { return a < b; }
template <typename A, typename B> inline auto pandac_less_eq(const A& a, const B& b) { return a <= b; }
template <typename A, typename B> inline auto pandac_greater(const A& a, const B& b) { return a > b; }
template <typename A, typename B> inline auto pandac_greater_eq(const A& a, const B& b) { return a >= b; }
template <typename A, typename B> inline auto pandac_and(const A& a, const B& b) { return a && b; }
template <typename A, typename B> inline auto pandac_or(const A& a, const B& b) { return a || b; }
template <typename A> inline auto pandac_negate(const A& a) { return !a; }

template <typename A, typename B> inline A& pandac_assign(A& lhs, const B& rhs) { 
    lhs = rhs; 
    return lhs; 
}

template <typename A, typename B> 
inline A& pandac_assign_add(A& lhs, const B& rhs) { 
    if constexpr (std::is_same_v<A, std::string> || std::is_same_v<B, std::string>) {
        lhs = pandac_str(lhs) + pandac_str(rhs);
    } else {
        lhs += rhs; 
    }
    return lhs; 
}

template <typename A, typename B> 
inline A& pandac_assign_sub(A& lhs, const B& rhs) { 
    lhs -= rhs; 
    return lhs; 
}

template <typename A, typename B> 
inline A& pandac_assign_mul(A& lhs, const B& rhs) { 
    if constexpr (std::is_same_v<A, std::string> && std::is_integral_v<B>) {
        std::string res; for(B i = 0; i < rhs; ++i) res += lhs; lhs = res;
    } else {
        lhs *= rhs; 
    }
    return lhs; 
}

template <typename A, typename B> inline A& pandac_assign_div(A& lhs, const B& rhs) { lhs /= rhs; return lhs; }
template <typename A, typename B> inline A& pandac_assign_int_div(A& lhs, const B& rhs) { lhs = static_cast<int64_t>(lhs) / static_cast<int64_t>(rhs); return lhs; }
template <typename A, typename B> inline A& pandac_assign_mod(A& lhs, const B& rhs) {
    if constexpr (std::is_integral_v<A> && std::is_integral_v<B>) lhs %= rhs;
    else lhs = std::fmod(static_cast<double>(lhs), static_cast<double>(rhs));
    return lhs;
}

}

using namespace PandaC;