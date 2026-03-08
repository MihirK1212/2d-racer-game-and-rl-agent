#include <algorithm>
#include <cmath>
#include <limits>

#include "collision.h"
#include "../../entity/car/car.h"
#include "../../entity/curve/curve.h"

static constexpr double WALL_RESTITUTION  = 0.3;
static constexpr double CAR_RESTITUTION   = 0.7;
static constexpr double SKIN_WIDTH        = 0.01; // small buffer to prevent surface oscillation


// ---------------------------------------------------------------------------
//  Car vs Curve  (rotated-rectangle vs parametric boundary)
// ---------------------------------------------------------------------------

CollisionResult detectCarVsCurve(Car* car, ParametricCurve2D* curve) {
    RectangleOBB obb = car->getOBB();

    // Which side of the curve is the car centre on?
    // For a CCW curve the outward normal points "away from centre of curvature".
    //   centerSide > 0  →  car is on the outward side  (inner boundary)
    //   centerSide < 0  →  car is on the inward  side  (outer boundary)
    ClosestPointResult centerClosest = curve->closestPointTo(car->getPosition());
    Vector2D centerNormal = curve->outwardNormalAt(centerClosest.theta);
    double centerSide = Vector2D::dot(
        car->getPosition() - centerClosest.position, centerNormal);

    // Build test points: 4 corners + 4 edge midpoints
    Vector2D testPoints[8];
    for (int i = 0; i < 4; ++i) testPoints[i] = obb.corners[i];
    for (int i = 0; i < 4; ++i)
        testPoints[4 + i] = (obb.corners[i] + obb.corners[(i + 1) % 4]) * 0.5;

    double   maxPen = 0.0;
    Vector2D collisionNormal;
    Vector2D contactPoint;
    bool     hit = false;

    for (int i = 0; i < 8; ++i) {
        ClosestPointResult cp = curve->closestPointTo(testPoints[i]);
        Vector2D normal = curve->outwardNormalAt(cp.theta);
        double signedDist = Vector2D::dot(testPoints[i] - cp.position, normal);

        double pen = 0.0;
        Vector2D pushDir;

        if (centerSide > 0) {
            // inner boundary: corner penetrates when it crosses inward
            if (signedDist < 0) { pen = -signedDist; pushDir = normal; }
        } else {
            // outer boundary: corner penetrates when it crosses outward
            if (signedDist > 0) { pen = signedDist; pushDir = normal * -1.0; }
        }

        if (pen > maxPen) {
            maxPen          = pen;
            collisionNormal = pushDir;
            contactPoint    = testPoints[i];
            hit             = true;
        }
    }

    if (!hit) return CollisionResult(false, Vector2D(), 0.0, Vector2D());
    return CollisionResult(true, collisionNormal, maxPen, contactPoint);
}


// ---------------------------------------------------------------------------
//  Car vs Car  (OBB–OBB via Separating-Axis Theorem)
// ---------------------------------------------------------------------------

CollisionResult detectCarVsCar(Car* car1, Car* car2) {
    RectangleOBB obb1 = car1->getOBB();
    RectangleOBB obb2 = car2->getOBB();

    Vector2D axes[4] = {
        obb1.axes[0], obb1.axes[1],
        obb2.axes[0], obb2.axes[1]
    };

    double   minOverlap = std::numeric_limits<double>::max();
    Vector2D minAxis;

    for (int a = 0; a < 4; ++a) {
        Vector2D axis = axes[a];

        double min1 =  std::numeric_limits<double>::max();
        double max1 = -std::numeric_limits<double>::max();
        double min2 =  std::numeric_limits<double>::max();
        double max2 = -std::numeric_limits<double>::max();

        for (int i = 0; i < 4; ++i) {
            double p1 = Vector2D::dot(obb1.corners[i], axis);
            double p2 = Vector2D::dot(obb2.corners[i], axis);
            min1 = std::min(min1, p1); max1 = std::max(max1, p1);
            min2 = std::min(min2, p2); max2 = std::max(max2, p2);
        }

        double overlap = std::min(max1, max2) - std::max(min1, min2);
        if (overlap <= 0)
            return CollisionResult(false, Vector2D(), 0.0, Vector2D());

        if (overlap < minOverlap) {
            minOverlap = overlap;
            minAxis    = axis;
        }
    }

    // Orient normal so it points from car1 toward car2
    Vector2D d = car2->getPosition() - car1->getPosition();
    if (Vector2D::dot(d, minAxis) < 0) minAxis = minAxis * -1.0;

    Vector2D contact = (car1->getPosition() + car2->getPosition()) * 0.5;
    return CollisionResult(true, minAxis, minOverlap, contact);
}


// ---------------------------------------------------------------------------
//  Resolution: Car vs Curve (wall)
// ---------------------------------------------------------------------------

void resolveCarVsCurve(Car* car, ParametricCurve2D* /*curve*/, CollisionResult& result) {
    if (!result.collided) return;

    // 1. Push the car out of the wall
    car->setPosition(car->getPosition() + result.normal * (result.penetration + SKIN_WIDTH));

    // 2. Remove the velocity component that goes into the wall
    //    v = speed * dir,  v_n = speed * dot(dir, n)
    //    After an inelastic-ish bounce the new speed along the car's own
    //    forward axis is:  speed' = speed * (1 - (1+e) * cos²α)
    //    where α = angle between car direction and wall normal.
    double velAlongNormal = Vector2D::dot(car->getVelocity(), result.normal);

    if (velAlongNormal < 0) {
        double cosA = Vector2D::dot(car->getDirection(), result.normal);
        double newSpeed = car->getSpeed() * (1.0 - (1.0 + WALL_RESTITUTION) * cosA * cosA);
        car->setSpeed(newSpeed);
    }
}


// ---------------------------------------------------------------------------
//  Resolution: Car vs Car (impulse, equal mass)
// ---------------------------------------------------------------------------

void resolveCarVsCar(Car* car1, Car* car2, CollisionResult& result) {
    if (!result.collided) return;

    // 1. Push apart equally
    Vector2D half = result.normal * ((result.penetration + SKIN_WIDTH) * 0.5);
    car1->setPosition(car1->getPosition() - half);
    car2->setPosition(car2->getPosition() + half);

    // 2. 1-D impulse along the collision normal (equal unit mass)
    Vector2D v1 = car1->getVelocity();
    Vector2D v2 = car2->getVelocity();
    double relVelN = Vector2D::dot(v1 - v2, result.normal);

    if (relVelN > 0) return; // already separating

    double j = -(1.0 + CAR_RESTITUTION) * relVelN * 0.5;
    Vector2D impulse = result.normal * j;

    Vector2D newV1 = v1 + impulse;
    Vector2D newV2 = v2 - impulse;

    // Project the new velocity vectors back onto each car's forward axis
    car1->setSpeed(Vector2D::dot(newV1, car1->getDirection()));
    car2->setSpeed(Vector2D::dot(newV2, car2->getDirection()));
}
