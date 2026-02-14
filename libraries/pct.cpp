#include <unordered_map>
#include <vector>
#include <string>
#include <variant>
#include <iostream>
#include <ranges>
#include <iomanip>

using IntCol = std::vector<int64_t>;
using DoubleCol = std::vector<double>;
using BoolCol = std::vector<int8_t>;
using StringCol = std::vector<std::string>;

using ColumnData = std::variant<IntCol, DoubleCol, BoolCol, StringCol>;

enum class DType { Int, Float, Boolean, String};

struct Column {
    std::string name;
    DType type;
    ColumnData data;

    BoolCol validity_mark;

    Column(std::string name_, DType type_, ColumnData data_)
        : name(std::move(name_)),
          type(type_),
          data(std::move(data_)),
          validity_mark(std::visit(
              [](const auto& vec) {
                  return BoolCol(vec.size(), true);
              }, data))
    {}

    [[nodiscard]] size_t size() const {
        return std::visit([](const auto& vec) {return vec.size();}, data);
    }

    void resize(size_t n) {
        std::visit([n](auto& vec) {vec.resize(n);}, data);
        validity_mark.resize(n, true);
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
public:

    DataFrame() = default;

    template <typename T>
    void add_column(const std::string& name, std::vector<T>&& values) {
        DType t;
        if constexpr (std::is_same_v<T, int64_t>) {
            t = DType::Int;
        } else if constexpr (std::is_same_v<T, double>) {
            t = DType::Float;
        } else if constexpr (std::is_same_v<T, int8_t>) {
            t = DType::Boolean;
        } else if constexpr (std::is_same_v<T, std::string>) {
            t = DType::String;
        } else {
            static_assert(std::is_same_v<T, void>, "Unsupported column type");
        }

        ColumnData cd = std::move(values);

        size_t col_size = std::visit([](const auto& v){ return v.size(); }, cd);
        if (columns.empty()) {
            n_rows = col_size;
        } else if (col_size != n_rows) {
            throw std::runtime_error("Shape mismatch: column length does not match existing data");
        }

        BoolCol validity(col_size, true);

        columns.emplace_back(name, t, std::move(cd));
        columns.back().validity_mark = std::move(validity);
    }

    void print(size_t rows = 5) const {
        rows = std::min(rows, n_rows);
        for (const auto& col : columns) {
            std::cout << std::setw(10) << col.name;
        }
        std::cout << '\n';

        for (size_t i = 0; i < rows; ++i) {
            for (const auto& col : columns) {
                std::visit(PrinterVisitor{i}, col.data);
            }
            std::cout << '\n';
        }
    }

    [[nodiscard]] double sum_column(size_t idx) const {
        if (idx >= columns.size()) throw std::runtime_error("Column index out of range");
        SumVisitor visitor;
        std::visit(visitor, columns[idx].data);
        return visitor.sum;
    }

    /*

    explicit DataFrame(const std::vector<std::vector<Cell>>& data,
          const std::vector<std::string>& indexes = {},
          const std::vector<std::string>& col_names = {},
          bool copy = true)
    {
        n_rows = data.size();
        row_indexes = indexes;

        size_t n_cols = n_rows > 0 ? data[0].size() : 0;

        if (!indexes.empty()) {
            if (indexes.size() != n_rows)
                throw std::runtime_error("row index size mismatch");
            row_indexes = indexes;
        } else {
            row_indexes.resize(n_rows);
            for (size_t i = 0; i < n_rows; ++i) row_indexes[i] = std::to_string(i);
        }

        for (size_t j = 0; j < n_cols; ++j) {
            std::string col_name = (j < col_names.size()) ? col_names[j] : std::to_string(j);
            Column col_data;
            col_data.reserve(n_rows);

            for (size_t i = 0; i < n_rows; ++i) {
                col_data.push_back(data[i][j]);
            }
            columns[col_name] = copy ? std::move(col_data) : col_data;
        }
    }

    [[nodiscard]] size_t size() const { return n_rows * columns.size(); }

    [[nodiscard]] size_t row_count() const { return n_rows; }

    [[nodiscard]] size_t column_count() const { return columns.size(); }

    [[nodiscard]] std::pair<size_t, size_t> shape() const {
        return {row_count(), column_count()};
    }

    void head(size_t n = 5) const {
        n = std::min(n, n_rows);
        for (size_t i = 0; i < n; ++i) {
            for (const auto& col : columns | std::views::values) {
                std::visit([](auto&& val) { std::cout << val << '\t'; }, col[i]);
            }
            std::cout << '\n';
        }
    }*/

private:
    std::vector<Column> columns;
    size_t n_rows = 0;
};