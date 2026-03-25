#include <iostream>
#include <string>
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

    template<typename U> requires std::convertible_to<U, T>
    Var(const Var<U>& other) : value(other.value) {}

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

    template<typename U> requires std::convertible_to<U, T>
    Var& operator=(const Var<U>& other) {
        value = static_cast<T>(other.value);
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
    Array() = default;

    template<typename U>
    Array(std::initializer_list<U> init) : data(init.begin(), init.end()) {}

    Var<size_t> len() const { return Var<size_t>(data.size()); }

    const T& operator [](size_t ind) const { return data[ind]; }
    T& operator [](size_t ind) { return Var<T>(data[ind]); }
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

inline void pandac_print() {
    std::cout << std::endl;
}

template <typename T, typename... Args>
void pandac_print(const T& first, const Args&... args) {
    std::cout << to_str(first);
    if constexpr (sizeof...(args) > 0) {
        std::cout << " ";
    }
    pandac_print(args...);
}

template <typename T>
std::string pandac_str(const T& val) {
    return to_str(val);
}

template <typename T>
int pandac_int(const T& val) {
    if constexpr (std::is_constructible_v<std::string, T> && !std::is_arithmetic_v<T>) {
        return std::stoi(std::string(val));
    } else {
        return static_cast<int>(val);
    }
}

template <typename T>
double pandac_float(const T& val) {
    if constexpr (std::is_constructible_v<std::string, T> && !std::is_arithmetic_v<T>) {
        return std::stod(std::string(val));
    } else {
        return static_cast<double>(val);
    }
}

template <typename T>
auto pandac_len(const T& val) {
    if constexpr (requires { val.size(); }) {
        return val.size();
    } else if constexpr (requires { val.len(); }) {
        return val.len();
    } else if constexpr (requires { val.length(); }) {
        return val.length();
    } else {
        return 0;
    }
}

template <typename T>
bool pandac_bool(const T& val) {
    if constexpr (std::is_constructible_v<std::string, T> && !std::is_arithmetic_v<T>) {
        return std::string(val).length() > 0;
    } else {
        return static_cast<bool>(val);
    }
}

struct PandaCDictProxy;
struct NestedProxy;

inline std::string unquote(std::string s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

struct NestedProxy {
    PandaCDictProxy* parent;
    std::string childKey;
    std::string cachedValue;

    NestedProxy(PandaCDictProxy* p, std::string k, std::string v) : parent(p), childKey(k), cachedValue(v) {}

    template<typename T>
    NestedProxy& operator=(const T& val); // Реализация отложена

    operator std::string() const { return unquote(cachedValue); }
    operator int() const { try { return std::stoi(unquote(cachedValue)); } catch(...) { return 0; } }
    operator double() const { try { return std::stod(unquote(cachedValue)); } catch(...) { return 0.0; } }

    bool operator==(const std::string& other) const { return unquote(cachedValue) == other; }
    bool operator==(const char* other) const { return unquote(cachedValue) == other; }
    bool operator==(int other) const { return (int)(*this) == other; }
    bool operator==(bool other) const {
        std::string v = unquote(cachedValue);
        bool isTrue = (v == "1" || v == "true" || v == "True");
        return isTrue == other;
    }
};

struct PandaCDictProxy {
    std::shared_ptr<std::unordered_map<std::string, std::string>> dictRef;
    std::string key;

    template<typename T>
    PandaCDictProxy& operator=(const T& val) {
        (*dictRef)[key] = pandac_str(val);
        return *this;
    }

    PandaCDictProxy& operator=(const std::string& val) {
        (*dictRef)[key] = val;
        return *this;
    }

    operator std::string() const { return (*dictRef)[key]; }
    operator int() const { try { return std::stoi((*dictRef)[key]); } catch(...) { return 0; } }
    operator double() const { try { return std::stod((*dictRef)[key]); } catch(...) { return 0.0; } }

    bool operator==(const std::string& other) const { return (*dictRef)[key] == other; }
    bool operator==(const char* other) const { return (*dictRef)[key] == other; }
    bool operator==(int other) const { return (int)(*this) == other; }
    bool operator==(bool other) const {
        std::string v = (*dictRef)[key];
        bool isTrue = (v == "1" || v == "true" || v == "True");
        return isTrue == other;
    }

    NestedProxy operator[](const std::string& childKey) {
        std::string raw = (*dictRef)[key];
        std::string keyPattern = "\"" + childKey + "\":";
        size_t pos = raw.find(keyPattern);

        if (pos == std::string::npos) return NestedProxy(this, childKey, "0");

        size_t valStart = pos + keyPattern.length();
        while (valStart < raw.length() && std::isspace(raw[valStart])) valStart++;

        size_t valEnd;
        if (raw[valStart] == '"') {
            valEnd = valStart + 1;
            while (valEnd < raw.length() && (raw[valEnd] != '"' || raw[valEnd-1] == '\\')) valEnd++;
            if (valEnd < raw.length()) valEnd++;
        } else {
            valEnd = raw.find_first_of(",}", valStart);
        }
        if (valEnd == std::string::npos) valEnd = raw.length();

        std::string val = raw.substr(valStart, valEnd - valStart);
        return NestedProxy(this, childKey, val);
    }
};

template<typename T>
NestedProxy& NestedProxy::operator=(const T& val) {
    std::string newVal = pandac_str(val);
    cachedValue = newVal;
    std::string raw = parent->operator std::string();
    std::string keyPattern = "\"" + childKey + "\":";
    size_t pos = raw.find(keyPattern);

    if (pos != std::string::npos) {
        size_t valStart = pos + keyPattern.length();
        while (valStart < raw.length() && std::isspace(raw[valStart])) valStart++;

        size_t valEnd;
        if (raw[valStart] == '"') {
            valEnd = valStart + 1;
            while (valEnd < raw.length() && (raw[valEnd] != '"' || raw[valEnd-1] == '\\')) valEnd++;
            if (valEnd < raw.length()) valEnd++;
        } else {
            valEnd = raw.find_first_of(",}", valStart);
        }
        if (valEnd == std::string::npos) valEnd = raw.length();

        std::string updated = raw.substr(0, valStart) + newVal + raw.substr(valEnd);
        *parent = updated;
    }
    return *this;
}

inline int operator-(const PandaCDictProxy& lhs, const PandaCDictProxy& rhs) { return (int)lhs - (int)rhs; }
inline int operator+(const PandaCDictProxy& lhs, const PandaCDictProxy& rhs) { return (int)lhs + (int)rhs; }
inline int operator*(const PandaCDictProxy& lhs, const PandaCDictProxy& rhs) { return (int)lhs * (int)rhs; }
inline int operator/(const PandaCDictProxy& lhs, const PandaCDictProxy& rhs) { return (int)lhs / (int)rhs; }

inline int operator-(const PandaCDictProxy& lhs, int rhs) { return (int)lhs - rhs; }
inline int operator-(int lhs, const PandaCDictProxy& rhs) { return lhs - (int)rhs; }
inline int operator+(const PandaCDictProxy& lhs, int rhs) { return (int)lhs + rhs; }
inline int operator+(int lhs, const PandaCDictProxy& rhs) { return lhs + (int)rhs; }

inline int operator-(const NestedProxy& lhs, int rhs) { return (int)lhs - rhs; }
inline int operator-(int lhs, const NestedProxy& rhs) { return lhs - (int)rhs; }
inline int operator+(const NestedProxy& lhs, int rhs) { return (int)lhs + rhs; }
inline int operator+(int lhs, const NestedProxy& rhs) { return lhs + (int)rhs; }

inline std::ostream& operator<<(std::ostream& os, const PandaCDictProxy& p) { return os << std::string(p); }
inline std::ostream& operator<<(std::ostream& os, const NestedProxy& p) { return os << p.cachedValue; }
inline std::string to_str(const PandaCDictProxy& p) { return std::string(p); }
inline std::string to_str(const NestedProxy& p) { return unquote(p.cachedValue); }

struct PandaCDict {
    std::shared_ptr<std::unordered_map<std::string, std::string>> data;

    PandaCDict() : data(std::make_shared<std::unordered_map<std::string, std::string>>()) {}

    PandaCDict(std::initializer_list<std::pair<std::string, std::string>> init)
        : data(std::make_shared<std::unordered_map<std::string, std::string>>()) {
        for (const auto& p : init) {
            (*data)[p.first] = p.second;
        }
    }

    template <typename T>
    PandaCDictProxy operator[](const T& key) {
        return PandaCDictProxy{data, pandac_str(key)};
    }
};

inline std::ostream& operator<<(std::ostream& os, const PandaCDict& d) {
    os << "{";
    bool first = true;
    for (const auto& item : *d.data) {
        if (!first) os << ", ";
        first = false;
        os << "\"" << item.first << "\": " << item.second;
    }
    os << "}";
    return os;
}
