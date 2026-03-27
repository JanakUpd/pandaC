namespace pct {
    class StringPool {
        std::unordered_map<std::string, size_t> string_to_id;
        std::vector<std::string> id_to_string;
    public:
        size_t intern(const std::string& s) {
            auto it = string_to_id.find(s);
            if (it != string_to_id.end()) return it->second;
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

    enum class DType { Int, Float, Boolean, String, Categorial };

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

        [[nodiscard]] size_t size() const {
            return std::visit([](const auto& vec) { return vec.size(); }, data);
        }

        void resize(size_t n) {
            std::visit([n](auto& vec) { vec.resize(n); }, data);
            validity.resize(n, true);
        }
    };

    class DataFrame {
    private:
        std::vector<Column> columns;
        std::vector<std::string> row_names;
        size_t n_rows = 0;

        void generate_row_names(size_t n) {
            row_names.clear();
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
            return static_cast<size_t>(std::ranges::distance(columns.begin(), it));
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
            columns.erase(columns.begin() + static_cast<std::ptrdiff_t>(get_column_index(name)));
            if (columns.empty()) n_rows = 0;
        }

        void rename(const std::string& old_name, const std::string& new_name) {
            columns[get_column_index(old_name)].name = new_name;
        }

        [[nodiscard]] DataFrame slice(size_t offset, size_t length) const {
            DataFrame df;
            if (offset >= n_rows) return df;

            size_t actual_len = std::min(length, n_rows - offset);
            df.n_rows = actual_len;

            auto start_it = row_names.begin() + static_cast<std::ptrdiff_t>(offset);
            df.row_names.assign(start_it, start_it + static_cast<std::ptrdiff_t>(actual_len));

            for (const auto& col : columns) {
                ColumnData new_data = std::visit([offset, actual_len]<typename T>(const T& arg) -> ColumnData {
                    auto it = arg.begin() + static_cast<std::ptrdiff_t>(offset);
                    return T(it, it + static_cast<std::ptrdiff_t>(actual_len));
                }, col.data);

                auto v_it = col.validity.begin() + static_cast<std::ptrdiff_t>(offset);
                BoolCol new_validity(v_it, v_it + static_cast<std::ptrdiff_t>(actual_len));

                Column new_col(col.name, col.type, std::move(new_data));
                new_col.validity = std::move(new_validity);
                df.columns.push_back(std::move(new_col));
            }
            return df;
        }

        [[nodiscard]] DataFrame take(const std::vector<size_t>& indices) const {
            DataFrame df;
            df.n_rows = indices.size();
            for (size_t idx : indices) df.row_names.push_back(row_names.at(idx));

            for (const auto& col : columns) {
                ColumnData new_data = std::visit([&indices]<typename T>(const T& arg) -> ColumnData {
                    T vec;
                    vec.reserve(indices.size());
                    for (size_t idx : indices) vec.push_back(arg.at(idx));
                    return vec;
                }, col.data);

                BoolCol new_validity;
                new_validity.reserve(indices.size());
                for (size_t idx : indices) new_validity.push_back(col.validity.at(idx));

                Column new_col(col.name, col.type, std::move(new_data));
                new_col.validity = std::move(new_validity);
                df.columns.push_back(std::move(new_col));
            }
            return df;
        }

        [[nodiscard]] DataFrame filter(const BoolCol& mask) const {
            if (mask.size() != n_rows) throw std::runtime_error("Mask size mismatch");
            std::vector<size_t> indices;
            for (size_t i = 0; i < n_rows; ++i) {
                if (mask[i]) indices.push_back(i);
            }
            return take(indices);
        }

        void vstack(const DataFrame& other) {
            if (width() != other.width()) throw std::runtime_error("Width mismatch");

            for (size_t i = 0; i < columns.size(); ++i) {
                if (columns[i].type != other.columns[i].type) throw std::runtime_error("Type mismatch");

                std::visit([]<typename T1, typename T2>(T1& arg, const T2& other_arg) {
                    if constexpr (std::is_same_v<T1, T2>) {
                        arg.insert(arg.end(), other_arg.begin(), other_arg.end());
                    }
                }, columns[i].data, other.columns[i].data);

                columns[i].validity.insert(columns[i].validity.end(),
                                           other.columns[i].validity.begin(),
                                           other.columns[i].validity.end());
            }
            row_names.insert(row_names.end(), other.row_names.begin(), other.row_names.end());
            n_rows += other.n_rows;
        }

        void with_row_count(const std::string& name = "row_nr") {
            IntCol ids(n_rows);
            std::iota(ids.begin(), ids.end(), 0);
            Column new_col(name, DType::Int, std::move(ids));
            columns.insert(columns.begin(), std::move(new_col));
        }
#define NUMERIC_VISITOR(COL_NAME, INIT_VAL, OP) \
const Column& col = get_column(COL_NAME); \
return std::visit([&](auto&& arg) -> double { \
using T = std::decay_t<decltype(arg)>; \
if constexpr (std::is_arithmetic_v<typename T::value_type>) { \
double res = INIT_VAL; \
for (size_t i = 0; i < arg.size(); ++i) { \
if (col.validity[i]) res = OP; \
} \
return res; \
} \
throw std::runtime_error("Numeric column required"); \
}, col.data);

        [[nodiscard]] double sum(const std::string& col_name) const {
            NUMERIC_VISITOR(col_name, 0.0, res + static_cast<double>(arg[i]))
        }

        [[nodiscard]] size_t count(const std::string& col_name) const {
            const Column& col = get_column(col_name);
            return static_cast<size_t>(std::ranges::count(col.validity, true));
        }

        [[nodiscard]] size_t n_unique(const std::string& col_name) const {
            const Column& col = get_column(col_name);
            return std::visit([&col]<typename T>(const T& arg) -> size_t {
                std::set<typename T::value_type> unique_vals;
                for(size_t i = 0; i < arg.size(); ++i) {
                    if (col.validity[i]) unique_vals.insert(arg[i]);
                }
                return unique_vals.size();
            }, col.data);
        }

        void glimpse() const {
            std::cout << "DataFrame: " << height() << " rows, " << width() << " columns\n";
            for (const auto& col : columns) {
                std::cout << std::left << std::setw(15) << col.name
                          << " [Type: " << static_cast<int>(col.type) << "]"
                          << " Nulls: " << (n_rows - static_cast<size_t>(std::ranges::count(col.validity, true)))
                          << "\n";
            }
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

#undef NUMERIC_VISITOR
    };
}
using namespace pct;