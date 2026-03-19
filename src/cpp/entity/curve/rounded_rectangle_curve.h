#ifndef CIRCULAR_CURVE_H
#define CIRCULAR_CURVE_H

#include "curve.h"

class RoundedRectangleCurve : public ParametricCurve2D {
    public:

    explicit RoundedRectangleCurve(float radius, float lengthStraightSection);

    protected:

    Vector2D evaluate(double thetaDegrees) const override;

    private:
    float radius;
    float lengthStraightSection;
};

#endif
