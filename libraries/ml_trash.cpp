#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>

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

int main() {return 0;}