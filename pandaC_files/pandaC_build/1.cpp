#include <concepts>
#include <iostream>
#include <iterator>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
//START OF BLOCK: pandaC

#include <vector>
template<typename T> struct Var;

template<typename T>
std::string to_str(const T &val) {
    if constexpr (std::is_convertible_v<T, std::string>) {
        return std::string(val);
    } else {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }
}

template<typename T> struct Array;
template<typename T> struct List;

template<typename T>
struct Var {
    T value;
    Var() : value(T{}) {}
    Var(const T &v) : value(v) {}

    template<typename U> requires (!std::is_same_v<std::decay_t<U>, T> && std::convertible_to<U, T>)
    Var(const U &v) : value(v) {}

    explicit Var(const Var<std::string> &v) requires (!std::is_same_v<T, std::string>) {
        std::stringstream ss(v.value);
        if (!(ss >> value))
            throw std::runtime_error("failed to parse '" + v.value + "' to target type.");
    }

    operator T &() { return value; }
    operator const T &() const { return value; }

    Var &operator=(const T &v) {
        value = v;
        return *this;
    }

    Var<std::string> substr(const Var<int>& start, const Var<int>& end) const
        requires std::is_same_v<T, std::string>;

    Array<std::string> split(const Var<std::string>& delimiter) const
        requires std::is_same_v<T, std::string>;
};

template<typename T>
struct Array {
    std::vector<T> data;

    Var<size_t> len() const { return Var<size_t>(data.size()); }

    const T& operator [](size_t ind) const { return data[ind]; }
    T& operator [](size_t ind) { return Var(data[ind]); }
    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
    ~Array() { data.clear(); }
};

template<typename T>
struct List {
    std::list<Var<T>> data;

    size_t len() const { return data.size(); }

    const T& operator [](size_t ind) {
        auto it = data.begin();
        std::advance(it, ind);
        return it->value;
    }
    ~List() { data.clear(); }
};

template<typename T>
struct Matr
{
    std::vector<std::vector<T>> data;

    Var<size_t> rows() const { return Var<size_t>(data.size()); }
    Var<size_t> cols() const { return data.empty() ? Var<size_t>(0) : Var<size_t>(data[0].size()); }

    const Var<T>& operator()(size_t row, size_t col) const { return data[row][col]; }
};
template<typename T>
Var<std::string> Var<T>::substr(const Var<int>& a, const Var<int>& b) const requires std::is_same_v<T, std::string> {
    Var<std::string> result = "";
    for (size_t it = 0; it < value.size(); ++it)
        if (it >= a.value && it < b.value)
             result.value.push_back(value[it]);
    return result;
}

template<typename T>
Array<std::string> Var<T>::split(const Var<std::string>& delimiter) const requires std::is_same_v<T, std::string> {
    Array<std::string> result;
    size_t start = 0;
    size_t pos = value.find(delimiter.value);
    while (pos != std::string::npos) {
        result.data.push_back(value.substr(start, pos - start));
        start = pos + delimiter.value.size();
        pos = value.find(delimiter.value, start);
    }
    result.data.push_back(value.substr(start));
    return result;
}

template<typename T>
std::ostream &operator<<(std::ostream &os, const Var<T> &v) {
    return os << v.value;
}

template<typename T>
Var<std::string> operator+(const Var<T> &lhs, const char *rhs) {
    return Var<std::string>(to_str(lhs.value) + std::string(rhs));
}

template<typename T>
Var<std::string> operator+(const char *lhs, const Var<T> &rhs) {
    return Var<std::string>(std::string(lhs) + to_str(rhs.value));
}

template<typename T>
Var<std::string> operator+(const Var<T> &lhs, const std::string &rhs) {
    return Var<std::string>(to_str(lhs.value) + rhs);
}

template<typename T>
Var<std::string> operator+(const std::string &lhs, const Var<T> &rhs) {
    return Var<std::string>(lhs + to_str(rhs.value));
}

template<typename T, typename U>
auto operator+(const Var<T> &lhs, const Var<U> &rhs) {
    if constexpr (std::is_same_v<T, std::string> || std::is_same_v<U, std::string>)
        return Var<std::string>(to_str(lhs.value) + to_str(rhs.value));
    else
        return Var<decltype(lhs.value + rhs.value)>(lhs.value + rhs.value);
}

template<typename T, typename U>
    requires (!std::is_same_v<U, const char *> && !std::is_convertible_v<U, std::string>)
auto operator+(const Var<T> &lhs, U rhs) {
    if constexpr (std::is_same_v<T, std::string>)
        return Var<std::string>(lhs.value + to_str(rhs));
    else
        return Var<decltype(lhs.value + rhs)>(lhs.value + rhs);
}

template<typename T, typename U>
    requires (!std::is_same_v<U, const char *> && !std::is_convertible_v<U, std::string>)
auto operator+(U lhs, const Var<T> &rhs) {
    if constexpr (std::is_same_v<T, std::string>)
        return Var<std::string>(to_str(lhs) + rhs.value);
    else
        return Var<decltype(lhs + rhs.value)>(lhs + rhs.value);
}

Var<std::string> input(Var<std::string> prompt = "") {
    if (prompt.value != "") std::cout << prompt.value;
    std::string s;
    std::getline(std::cin, s);
    return Var<std::string>(s);
}


std::vector<Var<size_t>> range(long long start, long long end, long long step = 1) {
    std::vector<Var<size_t>> result;
    result.reserve((end - start) / step);
    for (size_t i = start; i < end; i += step){
        result.push_back(i);
    }
    return result;
}

std::vector<Var<size_t>> range(long long end) {
    return range(0, end, 1);
}
//END OF BLOCK: pandaC

Var<int32_t> test(Var<int32_t> a, Var<int32_t> b) {
    return a + b;
}
int main() {
    Var<int32_t> result = test(5, 10);
    std::cout<<result + "s"<<' ';
}
