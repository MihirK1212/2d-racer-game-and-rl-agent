#include <algorithm>
#include <cmath>

#include "collision.h"
#include "../../entity/car/car.h"
#include "../../entity/curve/curve.h"


CollisionResult detectCarVsCurve(Car* car, ParametricCurve2D* curve) {
    CollisionResult cr(true, Vector2D(0, 0), 0.0, Vector2D(0, 0));
    return cr;
}

CollisionResult detectCarVsCar(Car* car1, Car* car2) {
    CollisionResult cr(true, Vector2D(0, 0), 0.0, Vector2D(0, 0));
    return cr;
}

void resolveCarVsCurve(Car* car, ParametricCurve2D* curve, CollisionResult& result) {
    return;
}

void resolveCarVsCar(Car* car1, Car* car2, CollisionResult& result) {
    return;
}