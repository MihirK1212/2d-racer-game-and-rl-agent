#ifndef COLLISION_H
#define COLLISION_H

#include "../vector.h"
#include "../rectangle_obb.h"
#include "../../entity/car/car.h"

struct CollisionResult {
    bool collided;
    Vector2D normal;        // collision surface normal (points away from obstacle)
    double penetration;     // how far the car penetrated
    Vector2D contactPoint;  // where the collision happened

    CollisionResult(bool collided_val, Vector2D normal_val, double penetration_val, Vector2D contactPoint_val) : collided(collided_val), normal(normal_val), penetration(penetration_val), contactPoint(contactPoint_val) {}
};

CollisionResult detectCarVsCurve(const RectangleOBB& car /* curve representation */);

CollisionResult detectCarVsCar(const RectangleOBB& carA, const RectangleOBB& carB);

void resolveCarVsWall(Car& car, const CollisionResult& result);
void resolveCarVsCar(Car& carA, Car& carB, const CollisionResult& result);

#endif