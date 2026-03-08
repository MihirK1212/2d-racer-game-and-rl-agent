#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

#include "curve.h"

Curve2DPoint ParametricCurve2D::getPointById(int pointId) const {
    auto it = idIndexMap.find(pointId);

    if (it == idIndexMap.end()) {
        throw std::out_of_range("Invalid pointId: " + std::to_string(pointId));
    }

    return points[it->second];
}

Curve2DPoint ParametricCurve2D::getPointForTheta(double thetaDegrees) const {

    if (points.empty()) {
        return {-1, thetaDegrees, evaluate(thetaDegrees)};
    }

    auto it = std::lower_bound(
        points.begin(),
        points.end(),
        thetaDegrees,
        [](const Curve2DPoint& p, double val) {
            return p.theta < val;
        }
    );

    if (it != points.end() && std::abs(it->theta - thetaDegrees) < epsilon) {
        return *it;
    }

    return {-1, thetaDegrees, evaluate(thetaDegrees)};
}

void ParametricCurve2D::generate(double startThetaDeg, double endThetaDeg, size_t numPoints) {

    if (numPoints < 2) {
        numPoints = 2;
    }

    points.clear();
    idIndexMap.clear();

    points.reserve(numPoints);
    idIndexMap.reserve(numPoints);

    const double step = (endThetaDeg - startThetaDeg) / (numPoints - 1);

    for (size_t i = 0; i < numPoints; ++i) {

        double theta = startThetaDeg + (i * step);

        Curve2DPoint point{
            static_cast<int>(i),
            theta,
            evaluate(theta)
        };

        points.push_back(point);
        idIndexMap[point.pointId] = i;
    }
}

const std::vector<Curve2DPoint>& ParametricCurve2D::getCachedPoints() const {
    return points;
}

Vector2D ParametricCurve2D::tangentAt(double thetaDegrees) const {
    constexpr double h = 0.01;
    Vector2D p1 = evaluate(thetaDegrees - h);
    Vector2D p2 = evaluate(thetaDegrees + h);
    return (p2 - p1).normalize();
}

Vector2D ParametricCurve2D::outwardNormalAt(double thetaDegrees) const {
    Vector2D t = tangentAt(thetaDegrees);
    return Vector2D(t.y, -t.x);
}

ClosestPointResult ParametricCurve2D::closestPointTo(const Vector2D& point) const {
    ClosestPointResult best;
    best.distance = 1e18;
    best.theta = 0;
    best.position = point;

    if (points.size() < 2) return best;

    for (size_t i = 0; i + 1 < points.size(); ++i) {
        const Vector2D& a = points[i].position;
        const Vector2D& b = points[i + 1].position;
        Vector2D ab = b - a;
        double lenSq = Vector2D::dot(ab, ab);

        double t = 0;
        if (lenSq > epsilon * epsilon) {
            t = Vector2D::dot(point - a, ab) / lenSq;
            t = std::clamp(t, 0.0, 1.0);
        }

        Vector2D closest = a + ab * t;
        double dist = (point - closest).magnitude();

        if (dist < best.distance) {
            best.distance = dist;
            best.position = closest;
            best.theta = points[i].theta + t * (points[i + 1].theta - points[i].theta);
        }
    }

    return best;
}
