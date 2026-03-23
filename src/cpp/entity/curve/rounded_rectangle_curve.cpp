#include <cmath>
#include "rounded_rectangle_curve.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

RoundedRectangleCurve::RoundedRectangleCurve(float radius, float lengthStraightSection)
    : radius(radius), lengthStraightSection(lengthStraightSection) {}

Vector2D RoundedRectangleCurve::evaluate(double thetaDegrees) const {
    // Normalize theta to [0, 360) and map it to distance along the stadium
    // perimeter, starting at the rightmost point and moving counter-clockwise.
    double theta = std::fmod(thetaDegrees, 360.0);
    if (theta < 0.0) {
        theta += 360.0;
    }

    const double R = radius;
    const double L = lengthStraightSection;
    const double quarterArc = M_PI * R * 0.5;
    const double halfArc = M_PI * R;
    const double perimeter = 2.0 * L + 2.0 * M_PI * R;
    const double s = (theta / 360.0) * perimeter;

    // 0 -> quarterArc: upper-right quarter arc
    if (s < quarterArc) {
        const double angle = s / R;
        return {(L * 0.5) + R * std::cos(angle), R * std::sin(angle)};
    }

    // quarterArc -> quarterArc + L: top straight
    if (s < quarterArc + L) {
        const double t = (s - quarterArc) / L;
        return {(L * 0.5) - t * L, R};
    }

    // quarterArc + L -> quarterArc + L + halfArc: left semicircle
    if (s < quarterArc + L + halfArc) {
        const double angle = (M_PI * 0.5) + (s - quarterArc - L) / R;
        return {-(L * 0.5) + R * std::cos(angle), R * std::sin(angle)};
    }

    // quarterArc + L + halfArc -> quarterArc + 2L + halfArc: bottom straight
    if (s < quarterArc + (2.0 * L) + halfArc) {
        const double t = (s - quarterArc - L - halfArc) / L;
        return {-(L * 0.5) + t * L, -R};
    }

    // Remaining distance: lower-right quarter arc back to the start point.
    const double angle = -(M_PI * 0.5) + (s - quarterArc - (2.0 * L) - halfArc) / R;
    return {(L * 0.5) + R * std::cos(angle), R * std::sin(angle)};
}