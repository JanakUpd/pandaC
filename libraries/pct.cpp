#include <unordered_map>
#include <vector>
#include <string>
#include <variant>
#include <iostream>
#include <ranges>
#include <iomanip>

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

struct PrinterVisitor {
    size_t index{};

    template<typename T>
    void operator()(const T& col) const {
        if constexpr (std::is_same_v<T, BoolCol>) {
            std::cout << std::setw(width) << (col[index] ? "true" : "false");
        } else {
            std::cout << std::setw(width) << col[index];
        }
    }

    int width = 10;
};

struct SumVisitor {
    double sum = 0.0;

    template<typename T>
    void operator()(const T& col) {
        using ValueType = T::value_type;

        if constexpr (std::is_arithmetic_v<ValueType>) {
            for (auto v : col)
                sum += static_cast<double>(v);
        } else {
            throw std::runtime_error("Cannot sum non-numeric column");
        }
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


    void print(size_t rows_limit = 10) const {
        if (columns.empty()) {
            std::cout << "DataFrame is empty.\n";
            return;
        }

        size_t to_show = std::min(rows_limit, n_rows);
        constexpr int col_w = 12;
        constexpr int row_w = 8;

        std::cout << std::left << std::setw(row_w) << "RowID" << " | ";
        for (const auto& col : columns) {
            std::cout << std::left << std::setw(col_w) << col.name;
        }
        std::cout << "\n" << std::string(row_w + 3 + (columns.size() * col_w), '-') << "\n";

        for (size_t i = 0; i < to_show; ++i) {
            std::cout << std::left << std::setw(row_w) << row_names[i] << " | ";

            for (const auto& col : columns) {
                std::visit(PrinterVisitor{i, col_w}, col.data);
            }
            std::cout << '\n';
        }
    }

    [[nodiscard]] const std::string& get_column_name(size_t idx) const { return columns.at(idx).name; }
    [[nodiscard]] const std::string& get_row_name(size_t idx) const { return row_names.at(idx); }
};