#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <cstdint>

using vd = std::vector<double>;
using vdd = std::vector<vd>;

class LogisticRegression {
private:
    vd weights;
    uint64_t m;
    double b;
    double sigmoid(double z) {
        return 1.0 / (1.0 + exp(-z));
    }
public:
    LogisticRegression(int features) {
        m = features;
        weights.resize(m);
        b = 0.0;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-0.01, 0.01);
        for (uint64_t i = 0; i < m; i++) weights[i] = dis(gen);
    }
    void fit(const vdd& X, const vd& y, uint64_t epochs, double learning_rate) {
        uint64_t n = X.size();
        vd dw(m, 0.0);
        double db = 0.0;
        for (uint64_t epoch = 0; epoch < epochs; epoch++) {
            vd dw(m, 0.0);
            double db = 0.0;
            for (int i = 0; i < n; i++) {
                double z = b;
                for (uint64_t j = 0; j < m; j++) z += weights[j] * X[i][j];
                double y_hat = sigmoid(z);
                double error = y_hat - y[i];
                for (int j = 0; j < m; j++) dw[j] += error * X[i][j];
                db += error;
            }
            for (int j = 0; j < m; j++) weights[j] -= learning_rate * dw[j] / n;
            b -= learning_rate * db / n;
        }
    }
    double predict_double(const vd& x) {
        double z = b;
        for (uint64_t j = 0; j < m; j++) z += weights[j] * x[j];
        return sigmoid(z);
    }
    int predict(const vd& x) {
        return predict_double(x) >= 0.5 ? 1 : 0;
    }
};

class ILoss {
public:
    virtual ~ILoss() = default;
    virtual double value(const vd& y_true, const vd& y_pred) const = 0;
    virtual vd gradient(const vd& y_true, const vd& y_pred) const = 0;
};

class BinaryCrossEntropy final : public ILoss {
public:
    double value(const vd& y_true, const vd& y_pred) const override {}
    vd gradient(const vd& y_true, const vd& y_pred) const override {}
};