#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <cstdint>

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

double dot(const Vector& a, const Vector& b) {
    if (a.size() != b.size()) throw std::invalid_argument("dot: size of vectors unequal");
    double result = 0.0;
    for (std::size_t i = 0; i < a.size(); i++) result += a[i] * b[i];
    return result;
}