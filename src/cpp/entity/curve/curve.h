#ifndef CURVE_H
#define CURVE_H

#include <vector>
#include <cmath>
#include <unordered_map>

#include "../../engine/vector.h"

struct Curve2DPoint {
    int pointId;
    double theta;
    Vector2D position;
};

class ParametricCurve2D {
    protected:

    virtual Vector2D evaluate(double thetaDegrees) const = 0;

    static constexpr double epsilon = 1e-6;
    static constexpr double PI = 3.14159265358979323846;

    // Primary storage (sorted by theta)
    std::vector<Curve2DPoint> points;

    // Fast lookup for pointId
    std::unordered_map<int, size_t> idIndexMap;

    public:

    virtual ~ParametricCurve2D() = default;

    Curve2DPoint getPointById(int pointId) const;

    Curve2DPoint getPointForTheta(double thetaDegrees) const;

    /** Regenerates entire cache. */
    void generate(double startThetaDeg, double endThetaDeg, size_t numPoints);

    const std::vector<Curve2DPoint>& getCachedPoints() const;
};

#endif
