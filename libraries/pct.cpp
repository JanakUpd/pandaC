#include <unordered_map>
#include <vector>
#include <string>
#include <variant>
#include <iostream>
#include <ranges>
#include <iomanip>
#include <execution>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <type_traits>

class StringPool {
    std::unordered_map<std::string, uint32_t> string_to_id;
    std::vector<std::string> id_to_string;
public:
    uint32_t intern(const std::string& s) {
        auto it = string_to_id.find(s);
        if (it != string_to_id.end()) return it->second;
        uint32_t id = id_to_string.size();
        id_to_string.push_back(s);
        string_to_id[s] = id;
        return id;
    }
    [[nodiscard]] const std::string& get(uint32_t id) const { return id_to_string.at(id); }
};

using IntCol = std::vector<int64_t>;
using DoubleCol = std::vector<double>;
using CatCol = std::vector<uint32_t>;
using BoolCol = std::vector<uint8_t>;

using ColumnData = std::variant<IntCol, DoubleCol, CatCol, BoolCol>;
enum class DType { Int, Float, Categorical, Bool };

struct Column {
    std::string name;
    DType type;
    ColumnData data;
    BoolCol validity;

    Column(std::string name_, DType type_, ColumnData data_, uint32_t n_rows)
        : name(std::move(name_)),
          type(type_),
          data(std::move(data_)),
          validity(n_rows, 1)
    {}

    [[nodiscard]] uint32_t size() const {
        return std::visit([](const auto& vec) { return vec.size(); }, data);
    }

    void resize(uint32_t n) {
        std::visit([n](auto& vec) { vec.resize(n); }, data);
        validity.resize(n, 1);
    }
};

template <typename Op, typename Lhs, typename Rhs>
struct BinaryExpr {
    const Lhs& lhs;
    const Rhs& rhs;
    [[nodiscard]] double evaluate(uint32_t i) const {
        return Op{}(lhs[i], rhs[i]);
    }
};

struct PrinterVisitor {
    uint32_t index{};
    int width = 10;

    template<typename T>
    void operator()(const T& col) const {
        if constexpr (std::is_same_v<T, BoolCol>) {
            std::cout << std::setw(width) << (col[index] ? "true" : "false");
        } else {
            std::cout << std::setw(width) << col[index];
        }
    }
};

struct SumVisitor {
    double sum = 0.0;

    template<typename T>
    void operator()(const T& col) {
        using ValueType = T::value_type;
        if constexpr (std::is_same_v<ValueType, int64_t> || std::is_same_v<ValueType, double>) {
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
    std::shared_ptr<StringPool> pool = std::make_shared<StringPool>();
    std::vector<std::string> row_names;
    uint32_t n_rows = 0;

    void ensure_consistency(uint32_t n) {
        if (columns.empty()) {
            n_rows = n;
            row_names.clear();
            row_names.reserve(n);
            for (uint32_t i = 0; i < n; ++i) row_names.push_back(std::to_string(i));
        } else if (n_rows != n) {
            throw std::runtime_error("Shape mismatch: expected " + std::to_string(n_rows) + " rows.");
        }
    }

public:
    DataFrame() = default;

    template <typename T>
    void add_column(std::vector<T>&& values, std::string name = "") {
        if (name.empty()) name = "col_" + std::to_string(columns.size());
        ensure_consistency(values.size());

        if constexpr (std::is_same_v<T, std::string>) {
            CatCol interned;
            interned.reserve(values.size());
            for (const auto& s : values) interned.push_back(pool->intern(s));
            columns.emplace_back(std::move(name), DType::Categorical, std::move(interned), n_rows);
        } else if constexpr (std::is_same_v<T, double>) {
            columns.emplace_back(std::move(name), DType::Float, std::move(values), n_rows);
        } else if constexpr (std::is_integral_v<T>) {
            IntCol int_vals(values.begin(), values.end());
            columns.emplace_back(std::move(name), DType::Int, std::move(int_vals), n_rows);
        } else {
            static_assert(sizeof(T) == 0, "Unsupported column type.");
        }
    }

    [[nodiscard]] double sum(const std::string& col_name) const {
        for (const auto& col : columns) {
            if (col.name == col_name) {
                if (col.type == DType::Float || col.type == DType::Int) {
                    SumVisitor visitor;
                    std::visit([&visitor](const auto& vec) {
                        visitor(vec);
                    }, col.data);
                    return visitor.sum;
                }
                throw std::runtime_error("Cannot sum non-numeric column");
            }
        }
        return 0.0;
    }

    void print(uint32_t limit = 10) const {
        if (columns.empty()) return;
        uint32_t show = std::min(limit, n_rows);
        constexpr int col_w = 12;
        constexpr int row_w = 8;

        std::cout << std::left << std::setw(row_w) << "RowID" << " | ";
        for (const auto& col : columns) std::cout << std::setw(col_w) << col.name;
        std::cout << "\n" << std::string(row_w + 3 + (columns.size() * col_w), '-') << "\n";

        for (uint32_t i = 0; i < show; ++i) {
            std::cout << std::left << std::setw(row_w) << row_names[i] << " | ";
            for (const auto& col : columns) {
                std::visit([&]<typename T>(const T& vec) {
                    if constexpr (std::is_same_v<T, CatCol>)
                        std::cout << std::setw(col_w) << pool->get(vec[i]);
                    else if constexpr (std::is_same_v<T, BoolCol>)
                        std::cout << std::setw(col_w) << (vec[i] ? "true" : "false");
                    else
                        std::cout << std::setw(col_w) << vec[i];
                }, col.data);
            }
            std::cout << "\n";
        }
    }
};