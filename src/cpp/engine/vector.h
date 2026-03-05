#ifndef VECTOR_H 
#define VECTOR_H

#include <cmath>

struct Vector2D {
    double x;
    double y;

    Vector2D(double x_val = 0.0, double y_val = 0.0) : x(x_val), y(y_val) {}

    Vector2D operator+(const Vector2D& other) const {
        return Vector2D(x + other.x, y + other.y);
    }

    Vector2D operator*(double scalar) const {
        return Vector2D(x * scalar, y * scalar);
    }

    double magnitude() const {
        return std::sqrt(std::pow(x, 2) + std::pow(y, 2));
    }
};

#endif