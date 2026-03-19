#include <cmath>

#include "rounded_rectangle_curve.h"

#include <cmath>
#include "rounded_rectangle_curve.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

RoundedRectangleCurve::RoundedRectangleCurve(float radius, float lengthStraightSection)
    : radius(radius), lengthStraightSection(lengthStraightSection) {}

Vector2D RoundedRectangleCurve::evaluate(double thetaDegrees) const {
    // Normalize theta to [0, 360)
    double theta = fmod(thetaDegrees, 360.0);
    if (theta < 0) theta += 360.0;

    double rad = theta * M_PI / 180.0;

    double R = radius;
    double L = lengthStraightSection;

    double x, y;

    // ---- Region 1: Right semicircle (0 → 90)
    if (theta >= 0 && theta < 90) {
        x = (L / 2.0) + R * cos(rad);
        y = R * sin(rad);
    }
    // ---- Region 2: Top straight (90 → 180)
    else if (theta >= 90 && theta < 180) {
        double t = (theta - 90.0) / 90.0; // 0 → 1
        x = (L / 2.0) - t * L;
        y = R;
    }
    // ---- Region 3: Left semicircle (180 → 270)
    else if (theta >= 180 && theta < 270) {
        x = -(L / 2.0) + R * cos(rad);
        y = R * sin(rad);
    }
    // ---- Region 4: Bottom straight (270 → 360)
    else {
        double t = (theta - 270.0) / 90.0; // 0 → 1
        x = -(L / 2.0) + t * L;
        y = -R;
    }

    return {x, y};
}