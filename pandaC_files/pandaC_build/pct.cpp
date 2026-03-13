#include <concepts>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>
//START OF BLOCK: pandaC

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
struct Array {
    std::vector<Var<T>> data;

    size_t len() const { return Var(data.size()); }

    const T& operator [](size_t ind) const { return data[ind].value; }
    Var<T>& operator [](size_t ind) { return data[ind]; }

    ~Array() { data.clear(); }
};

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
//END OF BLOCK: pandaC

//START OF BLOCK: pct

class StringPool {
    std::unordered_map<std::string, size_t> string_to_id;
    std::vector<std::string> id_to_string;
public:
    size_t intern(const std::string& s) {
        auto it = string_to_id.find(s);
        if (it!= string_to_id.end()) return it->second;
        size_t id = id_to_string.size();
        id_to_string.push_back(s);
        string_to_id[s] = id;
        return id;
    }
    [[nodiscard]] const std::string& get(size_t id) const { return id_to_string.at(id); }
};

using IntCol = std::vector<int64_t>;
using DoubleCol = std::vector<double>;
using BoolCol = std::vector<int8_t>;
using StringCol = std::vector<std::string>;
using CatCol = std::vector<size_t>;

using ColumnData = std::variant<IntCol, DoubleCol, BoolCol, StringCol, CatCol>;

enum class DType { Int, Float, Boolean, String, Categorial};

struct Column {
    std::string name;
    DType type;
    ColumnData data;

    BoolCol validity;

    Column(std::string name_, DType type_, ColumnData data_)
        : name(std::move(name_)),
          type(type_),
          data(std::move(data_)),
          validity(std::visit(
              [](const auto& vec) {
                  return BoolCol(vec.size(), true);
              }, data))
    {}

    template <typename Op, typename Lhs, typename Rhs>
struct BinaryExpr {
        const Lhs& lhs;
        const Rhs& rhs;
        [[nodiscard]] double evaluate(size_t i) const {
            return Op{}(lhs[i], rhs[i]);
        }
    };

    [[nodiscard]] size_t size() const {
        return std::visit([](const auto& vec) {return vec.size();}, data);
    }

    void resize(size_t n) {
        std::visit([n](auto& vec) {vec.resize(n);}, data);
        validity.resize(n, true);
    }
};

class DataFrame {
private:
    std::vector<Column> columns;
    std::vector<std::string> row_names;
    size_t n_rows = 0;

    void generate_row_names(size_t n) {
        row_names.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            row_names.push_back(std::to_string(i));
        }
    }

public:
    DataFrame() = default;

    template <typename T>
    void add_column(std::vector<T>&& values, std::string name = "") {
        if (name.empty()) {
            name = "column_" + std::to_string(columns.size());
        }

        DType t;
        if constexpr (std::is_same_v<T, int64_t>) t = DType::Int;
        else if constexpr (std::is_same_v<T, double>) t = DType::Float;
        else if constexpr (std::is_same_v<T, int8_t>) t = DType::Boolean;
        else if constexpr (std::is_same_v<T, std::string>) t = DType::String;
        else static_assert(std::is_same_v<T, void>, "Unsupported column type");

        size_t col_size = values.size();

        if (columns.empty()) {
            n_rows = col_size;
            generate_row_names(n_rows);
        } else if (col_size != n_rows) {
            throw std::runtime_error("Shape mismatch: column length does not match existing data");
        }

        columns.emplace_back(name, t, std::move(values));
    }

    [[nodiscard]] std::pair<size_t, size_t> shape() const { return {n_rows, columns.size()}; }

    [[nodiscard]] size_t height() const { return n_rows; }

    [[nodiscard]] size_t width() const { return columns.size(); }

    [[nodiscard]] bool is_empty() const { return n_rows == 0 || columns.empty(); }

    [[nodiscard]] std::vector<std::pair<std::string, DType>> schema() const {
        std::vector<std::pair<std::string, DType>> s;
        s.reserve(columns.size());
        for (const auto& col : columns) s.emplace_back(col.name, col.type);
            return s;
        }

    [[nodiscard]] size_t get_column_index(const std::string& name) const {
        auto it = std::ranges::find(columns, name, &Column::name);
        if (it == columns.end()) throw std::runtime_error("Column not found: " + name);
        return std::ranges::distance(columns.begin(), it);
    }

    [[nodiscard]] const Column& get_column(const std::string& name) const {
        return columns[get_column_index(name)];
    }

    [[nodiscard]] DataFrame select(const std::vector<std::string>& col_names) const {
        DataFrame df;
        df.n_rows = n_rows;
        df.row_names = row_names;
        for (const auto& name : col_names) df.columns.push_back(get_column(name));
        return df;
    }

    void drop(const std::string& name) {
        columns.erase(columns.begin() + get_column_index(name));
        if (columns.empty()) n_rows = 0;
    }

    void rename(const std::string& old_name, const std::string& new_name) {
        columns[get_column_index(old_name)].name = new_name;
    }

    void clear() {
        for (auto& col : columns) {
            std::visit([](auto&& arg) { arg.clear(); }, col.data);
            col.validity.clear();
        }
        row_names.clear();
        n_rows = 0;
    }

    [[nodiscard]] DataFrame clone() const {
        return *this;
    }

    [[nodiscard]] DataFrame slice(size_t offset, size_t length) const {
        DataFrame df;
        if (offset >= n_rows) return df;

        size_t actual_len = std::min(length, n_rows - offset);
        df.n_rows = actual_len;

        df.row_names.assign(row_names.begin() + offset, row_names.begin() + offset + actual_len);

        for (const auto& col : columns) {
            ColumnData new_data = std::visit([offset, actual_len]<typename T>(const T& arg) -> ColumnData {
                return T(arg.begin() + offset, arg.begin() + offset + actual_len);
            }, col.data);

            BoolCol new_validity(col.validity.begin() + offset, col.validity.begin() + offset + actual_len);

            Column new_col(col.name, col.type, std::move(new_data));
            new_col.validity = std::move(new_validity);
            df.columns.push_back(std::move(new_col));
        }
        return df;
    }

    [[nodiscard]] DataFrame head(size_t n = 5) const { return slice(0, n); }

    [[nodiscard]] DataFrame tail(size_t n = 5) const {
        return slice(n >= n_rows ? 0 : n_rows - n, n);
    }

    [[nodiscard]] DataFrame take(const std::vector<size_t>& indices) const {
        DataFrame df;
        df.n_rows = indices.size();
        for (size_t idx : indices) df.row_names.push_back(row_names.at(idx));

        for (const auto& col : columns) {

            ColumnData new_data = std::visit([&]<typename T>(const T& arg) -> ColumnData {
                T vec;
                vec.reserve(indices.size());
                for (size_t idx : indices) {
                    vec.push_back(arg.at(idx));
                }
                return vec;
            }, col.data);

            BoolCol new_validity; new_validity.reserve(indices.size());
            for (size_t idx : indices) new_validity.push_back(col.validity.at(idx));

            Column new_col(col.name, col.type, std::move(new_data));
            new_col.validity = std::move(new_validity);
            df.columns.push_back(std::move(new_col));
        }
        return df;
    }


    [[nodiscard]] DataFrame filter(const BoolCol& mask) const {
        if (mask.size() != n_rows) throw std::runtime_error("Mask size must match number of rows");
        std::vector<size_t> indices;
        for (size_t i = 0; i < n_rows; ++i) {
            if (mask[i]) indices.push_back(i);
        }
        return take(indices);
    }

    void print(size_t limit = 10) const {
        size_t to_show = std::min(limit, n_rows);
        for (const auto& col : columns) std::cout << std::setw(12) << col.name;
        std::cout << "\n" << std::string(columns.size() * 12, '-') << "\n";

        for (size_t i = 0; i < to_show; ++i) {
            for (const auto& col : columns) {
                std::visit([&](auto&& vec) {
                    std::cout << std::setw(12) << vec[i];
                }, col.data);
            }
            std::cout << "\n";
        }
    }


    [[nodiscard]] const std::string& get_column_name(size_t idx) const { return columns.at(idx).name; }
    [[nodiscard]] const std::string& get_row_name(size_t idx) const { return row_names.at(idx); }
};
//END OF BLOCK: pct

int main() {
    std::cout<<"PCT Library Test"<<'\n';
    Array<Var<std::string>> product_names;
    product_names.data = {"Laptop", "Mouse", "Monitor", "Keyboard", "HDMI Cable"};
    Array<Var<int32_t>> stock_levels;
    stock_levels.data = {15, 120, 45, 60, 200};
    Array<Var<double>> unit_prices;
    unit_prices.data = {1200.50, 25.00, 300.99, 45.00, 12.50};
    DataFrame inventory;
    inventory.add_column(product_names, "Product");
    inventory.add_column(stock_levels, "Stock");
    inventory.add_column(unit_prices, "Price");
    std::cout<<"Inventory Loaded: " + Var(inventory.rows()) + " rows, " + Var(inventory.cols()) + " columns"<<'\n';
    double total_valuation = 0.0;
    for (Var<int32_t> i = 0; i <  product_names.len(); ++i) {
        double item_total = Var<Var<double>>(stock_levels[i]) * unit_prices[i];
        total_valuation = total_valuation + item_total;
    }
    std::cout<<"Total Inventory Valuation: $" + Var(total_valuation)<<'\n';
    std::cout<<"Low Stock Alert (< 50 units)"<<'\n';
    for (Var<int32_t> i = 0; i <  stock_levels.len(); ++i) {
        if (stock_levels[i] < 50) {
            string alert_msg = " * " + product_names[i] + " (" + Var(stock_levels[i]) + ")";
            std::cout<<alert_msg<<'\n';
        }
    }
}
