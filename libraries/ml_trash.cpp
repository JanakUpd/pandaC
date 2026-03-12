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
    const std::size_t m = X[0].size();
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
    for (std::size_t i = 0; i < a.size(); i++) result += a[i] * b[i];
    return result;
}

Vector add(const Vector& a, const Vector& b) {
    if (a.size() != b.size()) throw std::invalid_argument("size of vectors in addiotion unequal");
    Vector result(a.size());
    for (std::size_t i = 0; i < a.size(); i++) result[i] = a[i] + b[i];
    return result;
}

Vector subtract(const Vector& a, const Vector& b) {
    if (a.size() != b.size()) throw std::invalid_argument("size of vectors in substriction unequal");
    Vector result(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) result[i] = a[i] - b[i];
    return result;
}

Vector multiply(const Vector& v, double scalar) {
    Vector result(v.size());
    for (std::size_t i = 0; i < v.size(); i++) result[i] = v[i] * scalar;
    return result;
}

Vector divide(const Vector& v, double scalar) {
    if (std::abs(scalar) < 1e-15) throw std::invalid_argument("scalar in dividition is zero");
    Vector result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = v[i] / scalar;
    return result;
}

int main() {return 0;}