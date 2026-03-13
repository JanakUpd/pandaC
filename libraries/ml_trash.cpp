#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <limits>
#include <numeric>

using Vector = std::vector<double>;
using Matrix = std::vector<Vector>;

void validate_matrix(const Matrix& X) {
    if (X.empty()) throw std::invalid_argument("Matrix is empty");
    const uint64_t m = X[0].size();
    if (m == 0) throw std::invalid_argument("Matrix has zero columns");
    for (const auto& row : X) {
        if (row.size() != m) throw std::invalid_argument("Matrix rows have different sizes");
    }
}

void validate_supervised_dataset(const Matrix& X, const Vector& y) {
    validate_matrix(X);
    if (X.size() != y.size()) throw std::invalid_argument("X and y size unequal");
}

double dot_product(const Vector& a, const Vector& b) {
    if (a.size() != b.size()) throw std::invalid_argument("size of vectors in dot_product unequal");
    double result = 0.0;
    for (uint64_t i = 0; i < a.size(); i++) result += a[i] * b[i];
    return result;
}

Vector add(const Vector& a, const Vector& b) {
    if (a.size() != b.size()) throw std::invalid_argument("size of vectors in addiotion unequal");
    Vector result(a.size());
    for (uint64_t i = 0; i < a.size(); i++) result[i] = a[i] + b[i];
    return result;
}

Vector subtract(const Vector& a, const Vector& b) {
    if (a.size() != b.size()) throw std::invalid_argument("size of vectors in substriction unequal");
    Vector result(a.size());
    for (uint64_t i = 0; i < a.size(); ++i) result[i] = a[i] - b[i];
    return result;
}

Vector multiply(const Vector& v, double scalar) {
    Vector result(v.size());
    for (uint64_t i = 0; i < v.size(); i++) result[i] = v[i] * scalar;
    return result;
}

Vector divide(const Vector& v, double scalar) {
    if (std::abs(scalar) < 1e-15) throw std::invalid_argument("scalar in dividition is zero");
    Vector result(v.size());
    for (uint64_t i = 0; i < v.size(); ++i) result[i] = v[i] / scalar;
    return result;
}

double squared_norm(const Vector& v) {return dot_product(v, v);}

double norm(const Vector& v) {return std::sqrt(squared_norm(v));}

double sigmoid(double z) {
    if (z >= 0.0) {
        const double e = std::exp(-z);
        return 1.0 / (1.0 + e);
    } else {
        const double e = std::exp(z);
        return e / (1.0 + e);
    }
}

Vector compute_column_mean(const Matrix& X) {
    validate_matrix(X);
    const uint64_t n = X.size();
    const uint64_t m = X[0].size();
    Vector mean(m, 0.0);
    for (const auto& row : X) {
        for (uint64_t j = 0; j < m; j++) mean[j] += row[j];
    }
    for (double& value : mean) value /= static_cast<double>(n);
    return mean;
}

Matrix center_matrix(const Matrix& X, const Vector& mean) {
    validate_matrix(X);
    if (X[0].size() != mean.size()) throw std::invalid_argument("size of mean and size of vector unequeal");
    Matrix centered = X;
    for (auto& row : centered) {
        for (uint64_t j = 0; j < mean.size(); ++j) row[j] -= mean[j];
    }
    return centered;
}

Matrix covariance_matrix(const Matrix& X_centered) {
    validate_matrix(X_centered);
    const uint64_t n = X_centered.size();
    const uint64_t m = X_centered[0].size();
    if (n < 2) throw std::invalid_argument("Need at least 2 samples for covariance matrix");
    Matrix cov(m, Vector(m, 0.0));
    for (uint64_t i = 0; i < n; i++) {
        for (uint64_t a = 0; a < m; a++) {
            for (uint64_t b = 0; b < m; b++) cov[a][b] += X_centered[i][a] * X_centered[i][b];
        }
    }
    for (uint64_t a = 0; a < m; ++a) {
        for (uint64_t b = 0; b < m; ++b) cov[a][b] /= static_cast<double>(n - 1);
    }
    return cov;
}

double majority_class(const Vector& y) {
    std::unordered_map<int, int> counts;
    for (double value : y) ++counts[static_cast<int>(std::round(value))];
    int best_class = 0;
    int best_count = -1;
    for (const auto& [cls, cnt] : counts) {
        if ((cnt > best_count) || (cnt == best_count && cls < best_class)) {
            best_class = cls;
            best_count = cnt;
        }
    }
    return static_cast<double>(best_class);
}

bool all_same_class(const Vector& y) {
    if (y.empty()) return true;
    const int first = static_cast<int>(std::round(y[0]));
    for (double value : y) {
        if (static_cast<int>(std::round(value)) != first) return false;
    }
    return true;
}


class IModel {
public:
    virtual ~IModel() = default;
};

class ISupervisedModel : public IModel {
public:
    virtual void fit(const Matrix& X, const Vector& y) = 0;
    virtual double predict(const Vector& x) const = 0;
    virtual ~ISupervisedModel() = default;
};

class IUnsupervisedModel : public IModel {
public:
    virtual void fit(const Matrix& X) = 0;
    virtual ~IUnsupervisedModel() = default;
};

class ITransformer : public IModel {
public:
    virtual void fit(const Matrix& X) = 0;
    virtual Matrix transform(const Matrix& X) const = 0;
    virtual ~ITransformer() = default;
};

class ILoss {
public:
    virtual ~ILoss() = default;
    virtual double value(const Vector& y_true, const Vector& y_pred) const = 0;
    virtual Vector gradient(const Vector& y_true, const Vector& y_pred) const = 0;
};

class IOptimizer {
public:
    virtual ~IOptimizer() = default;
    virtual void initialize(uint64_t param_count) = 0;
    virtual void step(Vector& params, const Vector& grads) = 0;
};

class IDistance {
public:
    virtual ~IDistance() = default;
    virtual double compute(const Vector& a, const Vector& b) const = 0;
};

class ISplitCriterion {
public:
    virtual ~ISplitCriterion() = default;
    virtual double impurity(const Vector& y) const = 0;
};

class MeanSquaredError final : public ILoss {
public:
    double value(const Vector& y_true, const Vector& y_pred) const override {
        if (y_true.size() != y_pred.size()) throw std::invalid_argument("MSE sizes unequal");
        double loss = 0.0;
        for (uint64_t i = 0; i < y_true.size(); i++) {
            const double diff = y_pred[i] - y_true[i];
            loss += diff * diff;
        }
        return loss / static_cast<double>(y_true.size());
    }
    Vector gradient(const Vector& y_true, const Vector& y_pred) const override {
        if (y_true.size() != y_pred.size()) throw std::invalid_argument("MSE gradient: sizes unequal");
        Vector grad(y_true.size(), 0.0);
        for (uint64_t i = 0; i < y_true.size(); i++) grad[i] = 2.0 * (y_pred[i] - y_true[i]);
        return grad;
    }
};

class BinaryCrossEntropy final : public ILoss {
public:
    double value(const Vector& y_true, const Vector& y_pred) const override {
        if (y_true.size() != y_pred.size()) throw std::invalid_argument("BCE sizes unequal");
        constexpr double eps = 1e-15;
        double loss = 0.0;
        for (uint64_t i = 0; i < y_true.size(); i++) {
            const double p = std::clamp(y_pred[i], eps, 1.0 - eps);
            loss += -y_true[i] * std::log(p) - (1.0 - y_true[i]) * std::log(1.0 - p);
        }
        return loss / static_cast<double>(y_true.size());
    }
    Vector gradient(const Vector& y_true, const Vector& y_pred) const override {
        if (y_true.size() != y_pred.size()) throw std::invalid_argument("BCE gradient: sizes unequal");
        constexpr double eps = 1e-15;
        Vector grad(y_true.size(), 0.0);
        for (uint64_t i = 0; i < y_true.size(); ++i) {
            const double p = std::clamp(y_pred[i], eps, 1.0 - eps);
            grad[i] = -(y_true[i] / p) + (1.0 - y_true[i]) / (1.0 - p);
        }
        return grad;
    }
};

class SGD final : public IOptimizer {
private:
    double learning_rate_;
public:
    explicit SGD(double learning_rate) : learning_rate_(learning_rate) {}
    void initialize(uint64_t) override {}
    void step(Vector& params, const Vector& grads) override {
        if (params.size() != grads.size()) throw std::invalid_argument("SGD: params/grads sizes unequal");
        for (uint64_t i = 0; i < params.size(); i++) params[i] -= learning_rate_ * grads[i];
    }
};

class Adam final : public IOptimizer {
private:
    double learning_rate_;
    double beta1_;
    double beta2_;
    double eps_;
    Vector m_;
    Vector v_;
    uint64_t t_ = 0;
public:
    Adam(double learning_rate = 0.001, double beta1 = 0.9, double beta2 = 0.999, double eps = 1e-8): learning_rate_(learning_rate), beta1_(beta1), beta2_(beta2), eps_(eps) {}
    void initialize(uint64_t param_count) override {
        m_.assign(param_count, 0.0);
        v_.assign(param_count, 0.0);
        t_ = 0;
    }
    void step(Vector& params, const Vector& grads) override {
        if (params.size() != grads.size()) throw std::invalid_argument("Adam: params/grads sizes unequal");
        ++t_;
        for (uint64_t i = 0; i < params.size(); ++i) {
            m_[i] = beta1_ * m_[i] + (1.0 - beta1_) * grads[i];
            v_[i] = beta2_ * v_[i] + (1.0 - beta2_) * grads[i] * grads[i];
            const double m_hat = m_[i] / (1.0 - std::pow(beta1_, static_cast<double>(t_)));
            const double v_hat = v_[i] / (1.0 - std::pow(beta2_, static_cast<double>(t_)));
            params[i] -= learning_rate_ * m_hat / (std::sqrt(v_hat) + eps_);
        }
    }
};

class EuclideanDistance final : public IDistance {
public:
    double compute(const Vector& a, const Vector& b) const override {
        if (a.size() != b.size()) throw std::invalid_argument("EuclideanDistance: sizes unequal");
        double sum = 0.0;
        for (uint64_t i = 0; i < a.size(); ++i) {
            const double diff = a[i] - b[i];
            sum += diff * diff;
        }
        return std::sqrt(sum);
    }
};

class GiniCriterion final : public ISplitCriterion {
public:
    double impurity(const Vector& y) const override {
        if (y.empty()) return 0.0;
        std::unordered_map<int, int> counts;
        for (double value : y) ++counts[static_cast<int>(std::round(value))];
        const double n = static_cast<double>(y.size());
        double sum_sq = 0.0;
        for (const auto& [cls, cnt] : counts) {
            const double p = cnt / n;
            sum_sq += p * p;
        }
        return 1.0 - sum_sq;
    }
};

class BaseGradientModel : public ISupervisedModel {
protected:
    Vector params_;
    std::unique_ptr<ILoss> loss_;
    std::unique_ptr<IOptimizer> optimizer_;
    virtual double activate(double z) const = 0;
    virtual double activation_derivative_from_output(double y_pred) const = 0;
    double linear_part(const Vector& x) const {
        const uint64_t feature_count = params_.size() - 1;
        if (x.size() != feature_count) throw std::invalid_argument("Input feature size unequals");
        double z = params_.back();
        for (uint64_t j = 0; j < feature_count; ++j) z += params_[j] * x[j];
        return z;
    }

public:
    BaseGradientModel(uint64_t feature_count, std::unique_ptr<ILoss> loss, std::unique_ptr<IOptimizer> optimizer) : params_(feature_count + 1, 0.0), loss_(std::move(loss)), optimizer_(std::move(optimizer)) {
        if (!loss_ || !optimizer_) throw std::invalid_argument("loss or optimizer is null");
        optimizer_->initialize(params_.size());
    }
    void fit(const Matrix& X, const Vector& y) override {fit(X, y, 1000, false);}
    void fit(const Matrix& X, const Vector& y, uint64_t epochs, bool verbose = false) {
        validate_supervised_dataset(X, y);
        const uint64_t n = X.size();
        const uint64_t m = X[0].size();
        if (params_.size() != m + 1) throw std::invalid_argument("Feature count unequals");
        for (uint64_t epoch = 0; epoch < epochs; ++epoch) {
            Vector y_pred(n, 0.0);
            for (uint64_t i = 0; i < n; i++) y_pred[i] = predict(X[i]);
            const Vector dL_dy = loss_->gradient(y, y_pred);
            Vector dL_dz(n, 0.0);
            for (uint64_t i = 0; i < n; i++) dL_dz[i] = dL_dy[i] * activation_derivative_from_output(y_pred[i]);
            Vector grads(m + 1, 0.0);
            for (uint64_t j = 0; j < m; ++j) {
                double grad_w = 0.0;
                for (uint64_t i = 0; i < n; i++) grad_w += dL_dz[i] * X[i][j];
                grads[j] = grad_w / static_cast<double>(n);
            }
            double grad_b = 0.0;
            for (double v : dL_dz) grad_b += v;
            grads[m] = grad_b / static_cast<double>(n);
            optimizer_->step(params_, grads);
            if (verbose && (epoch % 100 == 0 || epoch + 1 == epochs))std::cout << "Epoch " << epoch << " | loss = " << loss_->value(y, y_pred) << '\n';
        }
    }
    double predict(const Vector& x) const override {return activate(linear_part(x));}
    Vector weights() const {return Vector(params_.begin(), params_.end() - 1);}
    double bias() const {return params_.back();}
};

class LinearRegression final : public BaseGradientModel {
protected:
    double activate(double z) const override {return z;}
    double activation_derivative_from_output(double) const override {return 1.0;}
public:
    LinearRegression(uint64_t feature_count, std::unique_ptr<ILoss> loss,std::unique_ptr<IOptimizer> optimizer): BaseGradientModel(feature_count, std::move(loss), std::move(optimizer)) {}
};

class LogisticRegression final : public BaseGradientModel {
protected:
    double activate(double z) const override {return sigmoid(z);}

    double activation_derivative_from_output(double y_pred) const override {return y_pred * (1.0 - y_pred);}

public:
    LogisticRegression(uint64_t feature_count,std::unique_ptr<ILoss> loss,std::unique_ptr<IOptimizer> optimizer): BaseGradientModel(feature_count, std::move(loss), std::move(optimizer)) {}
    int predict_class(const Vector& x, double threshold = 0.5) const {return predict(x) >= threshold ? 1 : 0;}
};

class KNNClassifier final : public ISupervisedModel {
private:
    Matrix X_train_;
    Vector y_train_;
    uint64_t k_;
    std::unique_ptr<IDistance> distance_;
public:
    KNNClassifier(uint64_t k, std::unique_ptr<IDistance> distance): k_(k), distance_(std::move(distance)) {if (k_ == 0 || !distance_) throw std::invalid_argument("KNN: invalid parameters");}
    void fit(const Matrix& X, const Vector& y) override {
        validate_supervised_dataset(X, y);
        if (k_ > X.size()) throw std::invalid_argument("KNN: k > sample count");
        X_train_ = X;
        y_train_ = y;
    }
    double predict(const Vector& x) const override {
        if (X_train_.empty()) throw std::runtime_error("KNN is not fitted");
        std::vector<std::pair<double, int>> distances;
        distances.reserve(X_train_.size());
        for (uint64_t i = 0; i < X_train_.size(); ++i) {
            distances.emplace_back(
                distance_->compute(x, X_train_[i]),
                static_cast<int>(std::round(y_train_[i]))
            );
        }
        std::nth_element(
            distances.begin(),
            distances.begin() + static_cast<std::ptrdiff_t>(k_),
            distances.end(),
            [](const auto& lhs, const auto& rhs) {
                return lhs.first < rhs.first;
            }
        );
        std::unordered_map<int, int> votes;
        for (uint64_t i = 0; i < k_; ++i) ++votes[distances[i].second];
        int best_class = 0;
        int best_count = -1;
        for (const auto& [cls, cnt] : votes) {
            if (cnt > best_count || (cnt == best_count && cls < best_class)) {
                best_class = cls;
                best_count = cnt;
            }
        }
        return static_cast<double>(best_class);
    }
};

struct TreeNode {
    bool is_leaf = false;
    double prediction = 0.0;
    uint64_t feature_index = 0;
    double threshold = 0.0;
    std::unique_ptr<TreeNode> left;
    std::unique_ptr<TreeNode> right;
};

class DecisionTreeClassifier final : public ISupervisedModel {
private:
    std::unique_ptr<TreeNode> root_;
    std::unique_ptr<ISplitCriterion> criterion_;
    uint64_t max_depth_;
    uint64_t min_samples_split_;

    struct SplitResult {
        bool valid = false;
        uint64_t feature_index = 0;
        double threshold = 0.0;
        std::vector<uint64_t> left_indices;
        std::vector<uint64_t> right_indices;
        double score = std::numeric_limits<double>::infinity();
    };

    SplitResult find_best_split(const Matrix& X, const Vector& y) const {
        const uint64_t n = X.size();
        const uint64_t m = X[0].size();
        SplitResult best;
        for (uint64_t feature = 0; feature < m; ++feature) {
            std::vector<double> values;
            values.reserve(n);
            for (const auto& row : X) values.push_back(row[feature]);
            std::sort(values.begin(), values.end());
            values.erase(std::unique(values.begin(), values.end()), values.end());
            if (values.size() < 2) continue;
            for (uint64_t t = 0; t + 1 < values.size(); ++t) {
                const double threshold = 0.5 * (values[t] + values[t + 1]);
                std::vector<uint64_t> left_idx;
                std::vector<uint64_t> right_idx;
                for (uint64_t i = 0; i < n; ++i) {
                    if (X[i][feature] <= threshold) left_idx.push_back(i);
                    else right_idx.push_back(i);
                }
                if (left_idx.empty() || right_idx.empty()) continue;
                Vector y_left, y_right;
                y_left.reserve(left_idx.size());
                y_right.reserve(right_idx.size());
                for (uint64_t idx : left_idx) y_left.push_back(y[idx]);
                for (uint64_t idx : right_idx) y_right.push_back(y[idx]);
                const double n_left = static_cast<double>(y_left.size());
                const double n_right = static_cast<double>(y_right.size());
                const double n_total = static_cast<double>(n);
                const double score = (n_left / n_total) * criterion_->impurity(y_left) + (n_right / n_total) * criterion_->impurity(y_right);
                if (score < best.score) {
                    best.valid = true;
                    best.feature_index = feature;
                    best.threshold = threshold;
                    best.left_indices = std::move(left_idx);
                    best.right_indices = std::move(right_idx);
                    best.score = score;
                }
            }
        }
        return best;
    }
    std::unique_ptr<TreeNode> build_tree(const Matrix& X, const Vector& y, uint64_t depth) const {
        auto node = std::make_unique<TreeNode>();
        node->prediction = majority_class(y);
        if (y.empty() ||
            depth >= max_depth_ ||
            X.size() < min_samples_split_ ||
            all_same_class(y)) {
            node->is_leaf = true;
            return node;
        }
        SplitResult split = find_best_split(X, y);
        if (!split.valid) {
            node->is_leaf = true;
            return node;
        }
        Matrix X_left, X_right;
        Vector y_left, y_right;
        for (uint64_t idx : split.left_indices) {
            X_left.push_back(X[idx]);
            y_left.push_back(y[idx]);
        }
        for (uint64_t idx : split.right_indices) {
            X_right.push_back(X[idx]);
            y_right.push_back(y[idx]);
        }
        node->feature_index = split.feature_index;
        node->threshold = split.threshold;
        node->left = build_tree(X_left, y_left, depth + 1);
        node->right = build_tree(X_right, y_right, depth + 1);
        return node;
    }
public:
    DecisionTreeClassifier(std::unique_ptr<ISplitCriterion> criterion, uint64_t max_depth = 5, uint64_t min_samples_split = 2) : criterion_(std::move(criterion)), max_depth_(max_depth), min_samples_split_(min_samples_split) {
        if (!criterion_) throw std::invalid_argument("DecisionTree: criterion is null");
    }
    void fit(const Matrix& X, const Vector& y) override {
        validate_supervised_dataset(X, y);
        root_ = build_tree(X, y, 0);
    }
    double predict(const Vector& x) const override {
        if (!root_) throw std::runtime_error("DecisionTree is not fitted");
        const TreeNode* node = root_.get();
        while (!node->is_leaf) {
            if (x[node->feature_index] <= node->threshold) node = node->left.get();
            else node = node->right.get();
        }
        return node->prediction;
    }
};

class KMeans final : public IUnsupervisedModel {
private:
    uint64_t k_;
    uint64_t max_iters_;
    double tol_;
    Matrix centroids_;
    std::unique_ptr<IDistance> distance_;
    uint64_t nearest_centroid(const Vector& x) const {
        uint64_t best_idx = 0;
        double best_dist = std::numeric_limits<double>::infinity();
        for (uint64_t i = 0; i < centroids_.size(); ++i) {
            const double d = distance_->compute(x, centroids_[i]);
            if (d < best_dist) {
                best_dist = d;
                best_idx = i;
            }
        }
        return best_idx;
    }
public:
    KMeans(uint64_t k, std::unique_ptr<IDistance> distance, uint64_t max_iters = 100, double tol = 1e-6): k_(k), max_iters_(max_iters), tol_(tol), distance_(std::move(distance)) {
        if (k_ == 0 || !distance_) throw std::invalid_argument("KMeans: invalid parameters");
    }
    void fit(const Matrix& X) override {
        validate_matrix(X);
        if (k_ > X.size()) throw std::invalid_argument("KMeans: k > sample count");
        centroids_.assign(X.begin(), X.begin() + static_cast<std::ptrdiff_t>(k_));
        std::vector<uint64_t> assignments(X.size(), 0);
        for (uint64_t iter = 0; iter < max_iters_; ++iter) {
            for (uint64_t i = 0; i < X.size(); i++) assignments[i] = nearest_centroid(X[i]);
            Matrix new_centroids(k_, Vector(X[0].size(), 0.0));
            std::vector<uint64_t> counts(k_, 0);
            for (uint64_t i = 0; i < X.size(); i++) {
                const uint64_t cluster = assignments[i];
                for (uint64_t j = 0; j < X[i].size(); j++) new_centroids[cluster][j] += X[i][j];
                ++counts[cluster];
            }
            for (uint64_t c = 0; c < k_; c++) {
                if (counts[c] == 0) new_centroids[c] = centroids_[c];
                else for (uint64_t j = 0; j < new_centroids[c].size(); j++) new_centroids[c][j] /= static_cast<double>(counts[c]);
            }
            double shift = 0.0;
            for (uint64_t c = 0; c < k_; c++) shift += distance_->compute(centroids_[c], new_centroids[c]);
            centroids_ = std::move(new_centroids);
            if (shift < tol_) break;
        }
    }
    uint64_t predict_cluster(const Vector& x) const {
        if (centroids_.empty()) throw std::runtime_error("KMeans is not fitted");
        return nearest_centroid(x);
    }

    const Matrix& centroids() const {return centroids_;}
};



int main() {return 0;}