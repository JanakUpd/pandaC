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
        DType t;


        if (columns.empty()) {
            n_rows = col_size;
        } else if (col_size != n_rows) {
            throw std::runtime_error("Shape mismatch: column length does not match existing data");
        }


        }

        for (const auto& col : columns) {
        }

            for (const auto& col : columns) {
            }
            std::cout << '\n';
        }
    }

};