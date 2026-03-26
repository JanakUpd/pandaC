struct PandaVar;
struct PandaCList;
struct PandaCDict;


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

template <typename T>
std::string pandac_str(const T& val) {
    if constexpr (std::is_same_v<std::decay_t<T>, PandaVar>) return static_cast<std::string>(val);
    else return to_str(val);
}

template <typename T>
int pandac_int(const T& val) {
    if constexpr (std::is_same_v<std::decay_t<T>, PandaVar>) return static_cast<int>(val);
    else if constexpr (std::is_constructible_v<std::string, T> && !std::is_arithmetic_v<T>) return std::stoi(std::string(val));
    else return static_cast<int>(val);
}

template <typename T>
double pandac_float(const T& val) {
    if constexpr (std::is_same_v<std::decay_t<T>, PandaVar>) return static_cast<double>(val);
    else if constexpr (std::is_constructible_v<std::string, T> && !std::is_arithmetic_v<T>) return std::stod(std::string(val));
    else return static_cast<double>(val);
}

template <typename T>
bool pandac_bool(const T& val) {
    if constexpr (std::is_same_v<std::decay_t<T>, PandaVar>) return static_cast<bool>(val);
    else if constexpr (std::is_constructible_v<std::string, T> && !std::is_arithmetic_v<T>) return std::string(val).length() > 0;
    else return static_cast<bool>(val);
}

template <typename T>
auto pandac_len(const T& val) {
    if constexpr (std::is_same_v<std::decay_t<T>, PandaVar>) return val.length();
    else if constexpr (requires { val.size(); }) return val.size();
    else if constexpr (requires { val.len(); }) return val.len();
    else if constexpr (requires { val.length(); }) return val.length();
    else return 0;
}

inline std::string unquote(std::string s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

// ==========================================
// PANDAVAR CLASSES (Core Types)
// ==========================================

class PandaVar {
public:
    std::any data;

    PandaVar() = default;

    template<typename T>
    PandaVar(const T& val) : data(val) {}

    template<typename T>
    PandaVar& operator=(const T& val) {
        data = val;
        return *this;
    }

    template<typename K>
    PandaVar& pandac_getitem(const K& key);

    template<typename K>
    PandaVar& operator[](const K& key) {
        return pandac_getitem(key);
    }

    int length() const;

    std::vector<PandaVar>::iterator begin();
    std::vector<PandaVar>::iterator end();
    std::vector<PandaVar>::const_iterator begin() const;
    std::vector<PandaVar>::const_iterator end() const;

    friend PandaVar operator+(const PandaVar& a, const PandaVar& b) {
        if (a.data.type() == typeid(std::string) || b.data.type() == typeid(std::string))
            return PandaVar(static_cast<std::string>(a) + static_cast<std::string>(b));
        if (a.data.type() == typeid(double) || b.data.type() == typeid(double))
            return PandaVar(static_cast<double>(a) + static_cast<double>(b));
        return PandaVar(static_cast<int>(a) + static_cast<int>(b));
    }

    friend PandaVar operator-(const PandaVar& a, const PandaVar& b) {
        if (a.data.type() == typeid(double) || b.data.type() == typeid(double)) return PandaVar(static_cast<double>(a) - static_cast<double>(b));
        return PandaVar(static_cast<int>(a) - static_cast<int>(b));
    }

    friend PandaVar operator*(const PandaVar& a, const PandaVar& b) {
        if (a.data.type() == typeid(double) || b.data.type() == typeid(double)) return PandaVar(static_cast<double>(a) * static_cast<double>(b));
        return PandaVar(static_cast<int>(a) * static_cast<int>(b));
    }

    friend PandaVar operator/(const PandaVar& a, const PandaVar& b) {
        return PandaVar(static_cast<double>(a) / static_cast<double>(b));
    }

    explicit operator int() const {
        if (data.type() == typeid(int)) return std::any_cast<int>(data);
        if (data.type() == typeid(double)) return static_cast<int>(std::any_cast<double>(data));
        if (data.type() == typeid(std::string)) { try { return std::stoi(std::any_cast<std::string>(data)); } catch(...) { return 0; } }
        return 0;
    }

    explicit operator double() const {
        if (data.type() == typeid(double)) return std::any_cast<double>(data);
        if (data.type() == typeid(int)) return static_cast<double>(std::any_cast<int>(data));
        if (data.type() == typeid(std::string)) { try { return std::stod(std::any_cast<std::string>(data)); } catch(...) { return 0.0; } }
        return 0.0;
    }

    explicit operator bool() const {
        if (data.type() == typeid(bool)) return std::any_cast<bool>(data);
        if (data.type() == typeid(int)) return std::any_cast<int>(data) != 0;
        if (data.type() == typeid(std::string)) return std::any_cast<std::string>(data).length() > 0;
        return false;
    }

    explicit operator std::string() const;
    friend std::ostream& operator<<(std::ostream& os, const PandaVar& var);
};

struct PandaCDict {
    std::shared_ptr<std::unordered_map<std::string, PandaVar>> data;

    PandaCDict() : data(std::make_shared<std::unordered_map<std::string, PandaVar>>()) {}

    PandaCDict(std::initializer_list<std::pair<std::string, PandaVar>> init)
        : data(std::make_shared<std::unordered_map<std::string, PandaVar>>()) {
        for (const auto& p : init) {
            (*data)[p.first] = p.second;
        }
    }

    PandaVar& pandac_getitem(const std::string& key) {
        return (*data)[key];
    }
};

struct PandaCList {
    std::shared_ptr<std::vector<PandaVar>> data;

    PandaCList() : data(std::make_shared<std::vector<PandaVar>>()) {}

    PandaCList(std::initializer_list<PandaVar> init)
        : data(std::make_shared<std::vector<PandaVar>>(init.begin(), init.end())) {}

    PandaVar& pandac_getitem(int index) {
        return (*data)[index];
    }

    int length() const {
        return data->size();
    }
};

// ==========================================
// PANDAVAR METHOD IMPLEMENTATIONS
// ==========================================

template<typename K>
inline PandaVar& PandaVar::pandac_getitem(const K& key) {
    if (data.type() == typeid(PandaCDict)) {
        return std::any_cast<PandaCDict>(&data)->pandac_getitem(pandac_str(key));
    }
    if (data.type() == typeid(PandaCList)) {
        return std::any_cast<PandaCList>(&data)->pandac_getitem(pandac_int(key));
    }
    throw std::runtime_error("type does not support [index]");
}

inline int PandaVar::length() const {
    if (data.type() == typeid(PandaCList)) return std::any_cast<PandaCList>(&data)->length();
    if (data.type() == typeid(std::string)) return std::any_cast<std::string>(&data)->length();
    if (data.type() == typeid(PandaCDict)) return std::any_cast<PandaCDict>(&data)->data->size();
    return 0;
}

inline std::vector<PandaVar>::iterator PandaVar::begin() {
    if (data.type() == typeid(PandaCList)) return std::any_cast<PandaCList>(&data)->data->begin();
    throw std::runtime_error("Type is not iterable");
}
inline std::vector<PandaVar>::iterator PandaVar::end() {
    if (data.type() == typeid(PandaCList)) return std::any_cast<PandaCList>(&data)->data->end();
    throw std::runtime_error("Type is not iterable");
}
inline std::vector<PandaVar>::const_iterator PandaVar::begin() const {
    if (data.type() == typeid(PandaCList)) return std::any_cast<PandaCList>(&data)->data->begin();
    throw std::runtime_error("Type is not iterable");
}
inline std::vector<PandaVar>::const_iterator PandaVar::end() const {
    if (data.type() == typeid(PandaCList)) return std::any_cast<PandaCList>(&data)->data->end();
    throw std::runtime_error("Type is not iterable");
}

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

inline std::ostream& operator<<(std::ostream& os, const PandaCList& l) {
    os << "[";
    bool first = true;
    for (const auto& item : *l.data) {
        if (!first) os << ", ";
        first = false;
        os << item;
    }
    os << "]";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const PandaVar& var) {
    if (var.data.type() == typeid(PandaCList)) return os << std::any_cast<PandaCList>(var.data);
    if (var.data.type() == typeid(int)) return os << std::any_cast<int>(var.data);
    if (var.data.type() == typeid(double)) return os << std::any_cast<double>(var.data);
    if (var.data.type() == typeid(std::string)) return os << std::any_cast<std::string>(var.data);
    if (var.data.type() == typeid(PandaCDict)) return os << std::any_cast<PandaCDict>(var.data);
    if (var.data.type() == typeid(bool)) return os << (std::any_cast<bool>(var.data) ? "True" : "False");
    return os << "<PandaVar::None>";
}

inline PandaVar::operator std::string() const {
    if (data.type() == typeid(std::string)) return std::any_cast<std::string>(data);
    if (data.type() == typeid(int)) return std::to_string(std::any_cast<int>(data));
    if (data.type() == typeid(double)) return std::to_string(std::any_cast<double>(data));
    if (data.type() == typeid(PandaCDict)) {
        std::stringstream ss;
        ss << std::any_cast<PandaCDict>(data);
        return ss.str();
    }
    if (data.type() == typeid(PandaCList)) {
        std::stringstream ss;
        ss << std::any_cast<PandaCList>(data);
        return ss.str();
    }
    if (data.type() == typeid(bool)) return std::any_cast<bool>(data) ? "True" : "False";
    return "";
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

inline PandaVar input(PandaVar prompt = PandaVar("")) {
    if (std::string(prompt) != "") std::cout << std::string(prompt);
    std::string s;
    std::getline(std::cin, s);
    return PandaVar(s);
}

inline std::vector<PandaVar> range(long long start, long long end, long long step = 1) {
    std::vector<PandaVar> result;
    result.reserve((end - start) / step);
    for (long long i = start; i < end; i += step){
        result.push_back(PandaVar(static_cast<int>(i)));
    }
    return result;
}

inline std::vector<PandaVar> range(long long end) {
    return range(0, end, 1);
}


template<typename T>
bool pandac_negate(const T& a) {
    return !pandac_bool(a);
}

template<typename T, typename U>
auto pandac_assign(T&& lhs, const U& rhs) {
    lhs = rhs;
    return lhs;
}

template<typename T, typename U>
auto pandac_add(const T& a, const U& b) {
    if constexpr (std::is_same_v<std::decay_t<T>, PandaVar> || std::is_same_v<std::decay_t<U>, PandaVar>) {
        return a + b;
    } else if constexpr (std::is_constructible_v<std::string, decltype(a)> || std::is_constructible_v<std::string, decltype(b)>) {
        return pandac_str(a) + pandac_str(b);
    } else {
        return a + b;
    }
}

template<typename T, typename U>
auto pandac_sub(const T& a, const U& b) {
    return a - b;
}

template<typename T, typename U>
auto pandac_mul(const T& a, const U& b) {
    if constexpr (std::is_same_v<std::decay_t<T>, PandaVar> || std::is_same_v<std::decay_t<U>, PandaVar>) {
        return a * b;
    } else if constexpr (std::is_constructible_v<std::string, decltype(a)> && std::is_convertible_v<decltype(b), size_t>) {
        std::string res = "";
        for (size_t i = 0; i < pandac_int(b); ++i) res += pandac_str(a);
        return res;
    } else if constexpr (std::is_convertible_v<decltype(a), size_t> && std::is_constructible_v<std::string, decltype(b)>) {
        std::string res = "";
        for (size_t i = 0; i < pandac_int(a); ++i) res += pandac_str(b);
        return res;
    } else {
        return a * b;
    }
}

template<typename T, typename U>
auto pandac_div(const T& a, const U& b) {
    return pandac_float(a) / pandac_float(b);
}

template<typename T, typename U>
auto pandac_int_div(const T& a, const U& b) {
    return pandac_int(a) / pandac_int(b);
}

template<typename T, typename U>
auto pandac_mod(const T& a, const U& b) {
    return pandac_int(a) % pandac_int(b);
}

template<typename T, typename U>
auto pandac_pow(const T& a, const U& b) {
    if constexpr (std::is_same_v<std::decay_t<T>, PandaVar> || std::is_same_v<std::decay_t<U>, PandaVar>) {
        return PandaVar(std::pow(static_cast<double>(a), static_cast<double>(b)));
    } else {
        return std::pow(a, b);
    }
}

template<typename T, typename U>
auto pandac_and(const T& a, const U& b) {
    return pandac_bool(a) ? b : a;
}

template<typename T, typename U>
auto pandac_or(const T& a, const U& b) {
    return pandac_bool(a) ? a : b;
}

template<typename T, typename U>
auto pandac_assign_add(T&& lhs, const U& rhs) { return lhs = pandac_add(lhs, rhs); }

template<typename T, typename U>
auto pandac_assign_sub(T&& lhs, const U& rhs) { return lhs = pandac_sub(lhs, rhs); }

template<typename T, typename U>
auto pandac_assign_mul(T&& lhs, const U& rhs) { return lhs = pandac_mul(lhs, rhs); }

template<typename T, typename U>
auto pandac_assign_div(T&& lhs, const U& rhs) { return lhs = pandac_div(lhs, rhs); }

template<typename T, typename U>
auto pandac_assign_int_div(T&& lhs, const U& rhs) { return lhs = pandac_int_div(lhs, rhs); }

template<typename T, typename U>
auto pandac_assign_mod(T&& lhs, const U& rhs) { return lhs = pandac_mod(lhs, rhs); }

template<typename T, typename U>
auto pandac_eq(const T& a, const U& b) {
    if constexpr (std::is_same_v<std::decay_t<T>, PandaVar> || std::is_same_v<std::decay_t<U>, PandaVar>) {
        PandaVar pa(a); PandaVar pb(b);
        if (pa.data.type() == typeid(std::string) || pb.data.type() == typeid(std::string)) return PandaVar(static_cast<std::string>(pa) == static_cast<std::string>(pb));
        if (pa.data.type() == typeid(double) || pb.data.type() == typeid(double)) return PandaVar(static_cast<double>(pa) == static_cast<double>(pb));
        return PandaVar(static_cast<int>(pa) == static_cast<int>(pb));
    } else if constexpr (std::is_constructible_v<std::string, decltype(a)> && std::is_constructible_v<std::string, decltype(b)>) {
        return PandaVar(pandac_str(a) == pandac_str(b));
    } else {
        return PandaVar(pandac_float(a) == pandac_float(b));
    }
}

template<typename T, typename U>
auto pandac_neq(const T& a, const U& b) {
    return PandaVar(!pandac_bool(pandac_eq(a, b)));
}

template<typename T, typename U>
auto pandac_less(const T& a, const U& b) {
    if constexpr (std::is_same_v<std::decay_t<T>, PandaVar> || std::is_same_v<std::decay_t<U>, PandaVar>) {
        PandaVar pa(a); PandaVar pb(b);
        if (pa.data.type() == typeid(std::string) && pb.data.type() == typeid(std::string)) return PandaVar(static_cast<std::string>(pa) < static_cast<std::string>(pb));
        if (pa.data.type() == typeid(double) || pb.data.type() == typeid(double)) return PandaVar(static_cast<double>(pa) < static_cast<double>(pb));
        return PandaVar(static_cast<int>(pa) < static_cast<int>(pb));
    } else if constexpr (std::is_constructible_v<std::string, decltype(a)> && std::is_constructible_v<std::string, decltype(b)>) {
        return PandaVar(pandac_str(a) < pandac_str(b));
    } else {
        return PandaVar(pandac_float(a) < pandac_float(b));
    }
}

template<typename T, typename U>
auto pandac_less_eq(const T& a, const U& b) {
    if constexpr (std::is_same_v<std::decay_t<T>, PandaVar> || std::is_same_v<std::decay_t<U>, PandaVar>) {
        PandaVar pa(a); PandaVar pb(b);
        if (pa.data.type() == typeid(std::string) && pb.data.type() == typeid(std::string)) return PandaVar(static_cast<std::string>(pa) <= static_cast<std::string>(pb));
        if (pa.data.type() == typeid(double) || pb.data.type() == typeid(double)) return PandaVar(static_cast<double>(pa) <= static_cast<double>(pb));
        return PandaVar(static_cast<int>(pa) <= static_cast<int>(pb));
    } else if constexpr (std::is_constructible_v<std::string, decltype(a)> && std::is_constructible_v<std::string, decltype(b)>) {
        return PandaVar(pandac_str(a) <= pandac_str(b));
    } else {
        return PandaVar(pandac_float(a) <= pandac_float(b));
    }
}

template<typename T, typename U>
auto pandac_greater(const T& a, const U& b) {
    return PandaVar(!pandac_bool(pandac_less_eq(a, b)));
}

template<typename T, typename U>
auto pandac_greater_eq(const T& a, const U& b) {
    return PandaVar(!pandac_bool(pandac_less(a, b)));
}
