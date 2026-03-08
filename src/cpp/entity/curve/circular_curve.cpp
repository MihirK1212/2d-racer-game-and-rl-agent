#include <cmath>

#include "circular_curve.h"

CircularCurve::CircularCurve(float radius, double centerX, double centerY)
    : r(radius), centerX(centerX), centerY(centerY) {}

Vector2D CircularCurve::evaluate(double thetaDegrees) const {

    const double radians = thetaDegrees * (PI / 180.0);

    return {
        static_cast<float>(r * std::cos(radians)) + centerX,
        static_cast<float>(r * std::sin(radians)) + centerY
    };
}
