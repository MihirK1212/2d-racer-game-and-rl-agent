#ifndef CIRCULAR_CURVE_H
#define CIRCULAR_CURVE_H

#include "curve.h"

class CircularCurve : public ParametricCurve2D {
    public:

    explicit CircularCurve(float radius, double centerX, double centerY);

    protected:

    Vector2D evaluate(double thetaDegrees) const override;

    private:
    float r;
    double centerX;
    double centerY;
};

#endif
