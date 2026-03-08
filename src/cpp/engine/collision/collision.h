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

CollisionResult detectCarVsCurve(Car* car, ParametricCurve2D* curve);

CollisionResult detectCarVsCar(Car* car1, Car* car2);

void resolveCarVsCurve(Car* car, ParametricCurve2D* curve, CollisionResult& result);
void resolveCarVsCar(Car* car1, Car* car2, CollisionResult& result);

#endif