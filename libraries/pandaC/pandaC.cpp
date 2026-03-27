namespace PandaC {
    class PandaVar;

    class PandaObject {
    public:
        virtual ~PandaObject() = default;

        virtual std::string getClassName() const = 0;

        virtual PandaVar getProperty(const std::string &name);

        virtual void setProperty(const std::string &name, const PandaVar &value);

        virtual PandaVar &getItem(const PandaVar &key);

        virtual int64_t length() const { return 0; }

        virtual std::vector<PandaVar>::iterator begin() { throw std::runtime_error("Type is not iterable"); }
        virtual std::vector<PandaVar>::iterator end() { throw std::runtime_error("Type is not iterable"); }

        virtual std::vector<PandaVar>::const_iterator begin() const {
            throw std::runtime_error("Type is not iterable");
        }
        virtual std::vector<PandaVar>::const_iterator end() const { throw std::runtime_error("Type is not iterable"); }

        virtual std::string toString() const { return "<" + getClassName() + " object>"; }

        virtual PandaVar callMethod(const std::string &name, const std::vector<PandaVar> &args);
    };

    class PandaVar {
    public:
        using Callable = std::function<PandaVar(const std::vector<PandaVar>&)>;
        using Value = std::variant<
            std::monostate, int64_t, double, bool, std::string, std::shared_ptr<PandaObject>, Callable
        >;

        Value data;

        PandaVar pandac_call(const std::string& name, const std::vector<PandaVar>& args) {
            if (auto obj = std::get_if<std::shared_ptr<PandaObject>>(&data)) {
                return (*obj)->callMethod(name, args);
            }
            throw std::runtime_error("Attempted to call method '" + name + "' on non-object");
        }

        PandaVar callMethod(const std::string& name, const std::vector<PandaVar>& args) const {
            if (auto obj = std::get_if<std::shared_ptr<PandaObject>>(&data)) {
                return (*obj)->callMethod(name, args);
            }
            throw std::runtime_error("Attempted to call method '" + name + "' on non-object");
        }

        template<typename T, typename = std::enable_if_t<std::is_base_of_v<PandaObject, std::decay_t<T>>>>

        PandaVar(T&& obj) : data(std::shared_ptr<PandaObject>(new std::decay_t<T>(std::forward<T>(obj)))) {}
        PandaVar() : data(std::monostate{}) {}
        PandaVar(const char *v) : data(std::string(v)) {}
        PandaVar(const std::string &v) : data(v) {}
        PandaVar(int v) : data(static_cast<int64_t>(v)) {}
        PandaVar(int64_t v) : data(v) {}
        PandaVar(double v) : data(v) {}
        PandaVar(bool v) : data(v) {}
        PandaVar(std::shared_ptr<PandaObject> v) : data(std::move(v)) {}
        PandaVar(Callable f) : data(std::move(f)) {}

        operator std::vector<double>() const;
        operator std::vector<std::vector<double>>() const;

        template<typename K>
        PandaVar &pandac_getitem(const K &key);

        template<typename K>
        PandaVar &operator[](const K &key) { return pandac_getitem(key); }

        template<typename... Args>
        PandaVar operator()(Args&&... args) const {
            if (auto func = std::get_if<Callable>(&data)) {
                return (*func)({ PandaVar(std::forward<Args>(args))... });
            }
            throw std::runtime_error("Attempted to call a non-callable object");
        }

        PandaVar pandac_getprop(const std::string &name) const;

        void pandac_setprop(const std::string &name, const PandaVar &val);

        int64_t length() const;

        auto begin();
        auto end();
        auto begin() const;
        auto end() const;

        explicit operator int64_t() const;
        explicit operator double() const;
        explicit operator bool() const;
        explicit operator std::string() const;
    };

    inline PandaVar PandaObject::callMethod(const std::string &name, const std::vector<PandaVar> &args) {
        throw std::runtime_error("Method " + name + " not found on " + getClassName());
    }

    class PandaCDict : public PandaObject {
    public:
        std::unordered_map<std::string, PandaVar> data;

        PandaCDict() = default;

        PandaCDict(std::initializer_list<std::pair<const std::string, PandaVar>> init) {
            for (const auto &p: init) data[p.first] = p.second;
        }

        std::string getClassName() const override { return "dict"; }
        PandaVar &getItem(const PandaVar &key) override { return data[std::string(key)]; }
        int64_t length() const override { return static_cast<int64_t>(data.size()); }

        std::string toString() const override {
            std::string res = "{";
            bool first = true;
            for (const auto &item: data) {
                if (!first) res += ", ";
                first = false;
                res += "\"" + item.first + "\": " + std::string(item.second);
            }
            res += "}";
            return res;
        }
    };

    class Matr : public PandaObject {
    public:
        std::vector<std::vector<PandaVar>> data;

        Matr() = default;
        Matr(std::initializer_list<std::vector<PandaVar>> init) : data(init) {}

        std::string getClassName() const override { return "Matrix"; }

        PandaVar &getItem(const PandaVar &key) override {
            auto idx = static_cast<size_t>(static_cast<int64_t>(key));
            if (idx >= data.size()) throw std::runtime_error("Index out of bounds");
            return reinterpret_cast<PandaVar &>(data[idx]);
        }

        std::vector<PandaVar>::iterator begin() override {
            throw std::runtime_error("Direct iterator over Matr not supported via base interface");
        }

        std::vector<PandaVar>::iterator end() override {
            throw std::runtime_error("Direct iterator over Matr not supported via base interface");
        }

        std::vector<PandaVar>::const_iterator begin() const override {
            throw std::runtime_error("Direct iterator over Matr not supported via base interface");
        }

        std::vector<PandaVar>::const_iterator end() const override {
            throw std::runtime_error("Direct iterator over Matr not supported via base interface");
        }

        int64_t length() const override { return static_cast<int64_t>(data.size()); }

        std::string toString() const override {
            std::string res = "Matrix([";
            bool first = true;
            for (const auto &row: data) {
                if (!first) res += ", ";
                first = false;
                res += "[";
                bool first2 = true;
                for (const auto &item: row) {
                    if (!first2) res += ", ";
                    first2 = false;
                    res += std::string(item);
                }
                res += "]";
            }
            res += "])";
            return res;
        }

        PandaVar getProperty(const std::string &name) override {
            if (name == "rows") return static_cast<int64_t>(data.size());
            if (name == "cols") return data.empty() ? 0LL : static_cast<int64_t>(data[0].size());
            throw std::runtime_error("Property '" + name + "' not found in Matrix");
        }
    };

    class PandaCList : public PandaObject {
    public:
        std::vector<PandaVar> data;

        PandaCList() = default;
        PandaCList(std::initializer_list<PandaVar> init) : data(init) {}

        std::string getClassName() const override { return "list"; }

        PandaVar &getItem(const PandaVar &key) override {
            return data.at(static_cast<size_t>(static_cast<int64_t>(key)));
        }

        int64_t length() const override { return static_cast<int64_t>(data.size()); }
        std::vector<PandaVar>::iterator begin() override { return data.begin(); }
        std::vector<PandaVar>::iterator end() override { return data.end(); }
        std::vector<PandaVar>::const_iterator begin() const override { return data.begin(); }
        std::vector<PandaVar>::const_iterator end() const override { return data.end(); }

        std::string toString() const override {
            std::string res = "[";
            bool first = true;
            for (const auto &item: data) {
                if (!first) res += ", ";
                first = false;
                res += std::string(item);
            }
            res += "]";
            return res;
        }
    };

    inline PandaVar make_pandac_list(std::initializer_list<PandaVar> init) {
        return PandaVar(std::shared_ptr<PandaObject>(new PandaCList(init)));
    }

    inline PandaVar make_pandac_dict(std::initializer_list<std::pair<const std::string, PandaVar>> init) {
        return PandaVar(std::shared_ptr<PandaObject>(new PandaCDict(init)));
    }

    inline PandaVar PandaObject::getProperty(const std::string &name) {
        throw std::runtime_error("Property '" + name + "' not found in " + getClassName());
    }

    inline void PandaObject::setProperty(const std::string &name, const PandaVar &value) {
        throw std::runtime_error("Cannot set property '" + name + "' in " + getClassName());
    }

    inline PandaVar &PandaObject::getItem(const PandaVar &key) {
        throw std::runtime_error("Type " + getClassName() + " does not support indexing");
    }

    // --- PandaVar operations and conversions ---

    inline PandaVar::operator int64_t() const {
        if (auto p = std::get_if<int64_t>(&data)) return *p;
        if (auto p = std::get_if<double>(&data)) return static_cast<int64_t>(*p);
        if (auto p = std::get_if<bool>(&data)) return *p ? 1 : 0;
        if (auto p = std::get_if<std::string>(&data)) { try { return std::stoll(*p); } catch (...) { return 0; } }
        return 0;
    }

    inline PandaVar::operator double() const {
        if (auto p = std::get_if<double>(&data)) return *p;
        if (auto p = std::get_if<int64_t>(&data)) return static_cast<double>(*p);
        if (auto p = std::get_if<bool>(&data)) return *p ? 1.0 : 0.0;
        if (auto p = std::get_if<std::string>(&data)) { try { return std::stod(*p); } catch (...) { return 0.0; } }
        return 0.0;
    }

    inline PandaVar::operator bool() const {
        if (auto p = std::get_if<bool>(&data)) return *p;
        if (auto p = std::get_if<int64_t>(&data)) return *p != 0;
        if (auto p = std::get_if<double>(&data)) return *p != 0.0;
        if (auto p = std::get_if<std::string>(&data)) return !p->empty();
        if (std::holds_alternative<std::shared_ptr<PandaObject>>(data)) return length() != 0;
        return false;
    }

    inline PandaVar::operator std::string() const {
        if (auto p = std::get_if<std::string>(&data)) return *p;
        if (auto p = std::get_if<int64_t>(&data)) return std::to_string(*p);
        if (auto p = std::get_if<double>(&data)) return std::to_string(*p);
        if (auto p = std::get_if<bool>(&data)) return *p ? "True" : "False";
        if (auto p = std::get_if<std::shared_ptr<PandaObject>>(&data)) {
            if (*p) return (*p)->toString();
            return "None";
        }
        return "None";
    }

    template<typename K>
    inline PandaVar &PandaVar::pandac_getitem(const K &key) {
        if (auto obj = std::get_if<std::shared_ptr<PandaObject>>(&data)) return (*obj)->getItem(PandaVar(key));
        throw std::runtime_error("Type does not support indexing");
    }

    inline PandaVar PandaVar::pandac_getprop(const std::string &name) const {
        if (auto obj = std::get_if<std::shared_ptr<PandaObject>>(&data)) return (*obj)->getProperty(name);
        throw std::runtime_error("Attempted to read property on non-object");
    }

    inline void PandaVar::pandac_setprop(const std::string &name, const PandaVar &val) {
        if (auto obj = std::get_if<std::shared_ptr<PandaObject>>(&data)) {
            (*obj)->setProperty(name, val);
            return;
        }
        throw std::runtime_error("Attempted to set property on non-object");
    }

    inline int64_t PandaVar::length() const {
        if (auto obj = std::get_if<std::shared_ptr<PandaObject>>(&data)) return (*obj)->length();
        if (auto s = std::get_if<std::string>(&data)) return static_cast<int64_t>(s->size());
        return 0;
    }

    inline auto PandaVar::begin() {
        if (auto obj = std::get_if<std::shared_ptr<PandaObject>>(&data)) return (*obj)->begin();
        throw std::runtime_error("Type is not iterable");
    }

    inline auto PandaVar::end() {
        if (auto obj = std::get_if<std::shared_ptr<PandaObject>>(&data)) return (*obj)->end();
        throw std::runtime_error("Type is not iterable");
    }

    inline auto PandaVar::begin() const {
        if (auto obj = std::get_if<std::shared_ptr<PandaObject>>(&data)) return (*obj)->begin();
        throw std::runtime_error("Type is not iterable");
    }

    inline auto PandaVar::end() const {
        if (auto obj = std::get_if<std::shared_ptr<PandaObject>>(&data)) return (*obj)->end();
        throw std::runtime_error("Type is not iterable");
    }

    inline PandaVar::operator std::vector<double>() const {
        std::vector<double> res;
        if (std::holds_alternative<std::shared_ptr<PandaObject>>(data)) {
            for (const auto &item: *this) {
                res.push_back(static_cast<double>(item));
            }
        }
        return res;
    }

    inline PandaVar::operator std::vector<std::vector<double>>() const {
        std::vector<std::vector<double>> res;
        if (std::holds_alternative<std::shared_ptr<PandaObject>>(data)) {
            for (const auto &row: *this) {
                res.push_back(static_cast<std::vector<double>>(row));
            }
        }
        return res;
    }

    inline PandaVar operator+(const PandaVar &a, const PandaVar &b) {
        if (std::holds_alternative<std::string>(a.data) || std::holds_alternative<std::string>(b.data)) {
            return PandaVar(std::string(a) + std::string(b));
        }
        if (std::holds_alternative<double>(a.data) || std::holds_alternative<double>(b.data)) {
            return PandaVar(static_cast<double>(a) + static_cast<double>(b));
        }
        return PandaVar(static_cast<int64_t>(a) + static_cast<int64_t>(b));
    }

    inline PandaVar operator-(const PandaVar &a, const PandaVar &b) {
        if (std::holds_alternative<double>(a.data) || std::holds_alternative<double>(b.data)) {
            return PandaVar(static_cast<double>(a) - static_cast<double>(b));
        }
        return PandaVar(static_cast<int64_t>(a) - static_cast<int64_t>(b));
    }

    inline PandaVar operator*(const PandaVar &a, const PandaVar &b) {
        if (std::holds_alternative<double>(a.data) || std::holds_alternative<double>(b.data)) {
            return PandaVar(static_cast<double>(a) * static_cast<double>(b));
        }
        return PandaVar(static_cast<int64_t>(a) * static_cast<int64_t>(b));
    }

    inline PandaVar operator/(const PandaVar &a, const PandaVar &b) {
        return PandaVar(static_cast<double>(a) / static_cast<double>(b));
    }

    inline std::ostream &operator<<(std::ostream &os, const PandaVar &var) {
        os << std::string(var);
        return os;
    }

    template<typename T>
    std::string to_str(const T &val) { return std::string(PandaVar(val)); }

    inline std::string pandac_str(const PandaVar &val) { return std::string(val); }
    inline int64_t pandac_int64(const PandaVar &val) { return static_cast<int64_t>(val); }
    inline double pandac_float(const PandaVar &val) { return static_cast<double>(val); }
    inline bool pandac_bool(const PandaVar &val) { return static_cast<bool>(val); }
    inline int64_t pandac_len(const PandaVar &val) { return val.length(); }

    inline void pandac_print() { std::cout << std::endl; }

    template<typename T, typename... Args>
    void pandac_print(const T &first, const Args &... args) {
        std::cout << PandaVar(first);
        if constexpr (sizeof...(args) > 0) {
            std::cout << " ";
            pandac_print(args...);
        } else { std::cout << std::endl; }
    }

    inline PandaVar input(PandaVar prompt = PandaVar("")) {
        if (std::string(prompt) != "") std::cout << std::string(prompt);
        std::string s;
        std::getline(std::cin, s);
        return PandaVar(s);
    }

    inline PandaVar pandac_add(const PandaVar &a, const PandaVar &b) { return a + b; }
    inline PandaVar pandac_sub(const PandaVar &a, const PandaVar &b) { return a - b; }
    inline PandaVar pandac_mul(const PandaVar &a, const PandaVar &b) { return a * b; }
    inline PandaVar pandac_div(const PandaVar &a, const PandaVar &b) { return a / b; }

    inline PandaVar pandac_int64_div(const PandaVar &a, const PandaVar &b) {
        return PandaVar(static_cast<int64_t>(a) / static_cast<int64_t>(b));
    }

    inline PandaVar pandac_mod(const PandaVar &a, const PandaVar &b) {
        return PandaVar(static_cast<int64_t>(a) % static_cast<int64_t>(b));
    }

    inline PandaVar pandac_pow(const PandaVar &a, const PandaVar &b) {
        return PandaVar(std::pow(static_cast<double>(a), static_cast<double>(b)));
    }

    inline PandaVar pandac_eq(const PandaVar &a, const PandaVar &b) {
        if (std::holds_alternative<std::string>(a.data) || std::holds_alternative<std::string>(b.data)) {
            return PandaVar(std::string(a) == std::string(b));
        }
        if (std::holds_alternative<double>(a.data) || std::holds_alternative<double>(b.data)) {
            return PandaVar(static_cast<double>(a) == static_cast<double>(b));
        }
        return PandaVar(static_cast<int64_t>(a) == static_cast<int64_t>(b));
    }

    inline PandaVar pandac_neq(const PandaVar &a, const PandaVar &b) {
        return PandaVar(!static_cast<bool>(pandac_eq(a, b)));
    }

    inline PandaVar pandac_less(const PandaVar &a, const PandaVar &b) {
        if (std::holds_alternative<std::string>(a.data) && std::holds_alternative<std::string>(b.data)) {
            return PandaVar(std::string(a) < std::string(b));
        }
        if (std::holds_alternative<double>(a.data) || std::holds_alternative<double>(b.data)) {
            return PandaVar(static_cast<double>(a) < static_cast<double>(b));
        }
        return PandaVar(static_cast<int64_t>(a) < static_cast<int64_t>(b));
    }

    inline PandaVar pandac_less_eq(const PandaVar &a, const PandaVar &b) {
        if (std::holds_alternative<std::string>(a.data) && std::holds_alternative<std::string>(b.data)) {
            return PandaVar(std::string(a) <= std::string(b));
        }
        if (std::holds_alternative<double>(a.data) || std::holds_alternative<double>(b.data)) {
            return PandaVar(static_cast<double>(a) <= static_cast<double>(b));
        }
        return PandaVar(static_cast<int64_t>(a) <= static_cast<int64_t>(b));
    }

    inline PandaVar pandac_greater(const PandaVar &a, const PandaVar &b) {
        return PandaVar(!static_cast<bool>(pandac_less_eq(a, b)));
    }

    inline PandaVar pandac_greater_eq(const PandaVar &a, const PandaVar &b) {
        return PandaVar(!static_cast<bool>(pandac_less(a, b)));
    }

    inline PandaVar pandac_and(const PandaVar &a, const PandaVar &b) { return static_cast<bool>(a) ? b : a; }
    inline PandaVar pandac_or(const PandaVar &a, const PandaVar &b) { return static_cast<bool>(a) ? a : b; }
    inline PandaVar pandac_negate(const PandaVar &a) { return PandaVar(!static_cast<bool>(a)); }

    inline PandaVar &pandac_assign(PandaVar &lhs, const PandaVar &rhs) {
        lhs = rhs;
        return lhs;
    }

    inline PandaVar &pandac_assign_add(PandaVar &lhs, const PandaVar &rhs) {
        lhs = pandac_add(lhs, rhs);
        return lhs;
    }

    inline PandaVar &pandac_assign_sub(PandaVar &lhs, const PandaVar &rhs) {
        lhs = pandac_sub(lhs, rhs);
        return lhs;
    }

    inline PandaVar &pandac_assign_mul(PandaVar &lhs, const PandaVar &rhs) {
        lhs = pandac_mul(lhs, rhs);
        return lhs;
    }

    inline PandaVar &pandac_assign_div(PandaVar &lhs, const PandaVar &rhs) {
        lhs = pandac_div(lhs, rhs);
        return lhs;
    }

    inline PandaVar &pandac_assign_int_div(PandaVar &lhs, const PandaVar &rhs) {
        lhs = pandac_int64_div(lhs, rhs);
        return lhs;
    }

    inline PandaVar &pandac_assign_mod(PandaVar &lhs, const PandaVar &rhs) {
        lhs = pandac_mod(lhs, rhs);
        return lhs;
    }

    struct PandaRange {
        int64_t start_;
        int64_t end_;
        int64_t step_;

        PandaRange(int64_t s, int64_t e, int64_t st) : start_(s), end_(e), step_(st) {}

        struct Iterator {
            int64_t current_;
            int64_t step_;

            Iterator(int64_t current, int64_t step) : current_(current), step_(step) {}

            PandaVar operator*() const { return PandaVar(current_); }

            Iterator &operator++() {
                current_ += step_;
                return *this;
            }

            bool operator!=(const Iterator &other) const {
                if (step_ > 0) return current_ < other.current_;
                return current_ > other.current_;
            }
        };

        Iterator begin() const { return Iterator(start_, step_); }
        Iterator end() const { return Iterator(end_, step_); }
    };

    inline PandaRange range(long long start, long long end, long long step = 1) { return PandaRange(start, end, step); }
    inline PandaRange range(long long end) { return PandaRange(0, end, 1); }

}

using namespace PandaC;
