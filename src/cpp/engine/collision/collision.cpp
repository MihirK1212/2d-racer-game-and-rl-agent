#include <algorithm>
#include <cmath>

#include "collision.h"


CollisionResult detectCarVsCurve(const RectangleOBB& car /* curve representation */) {
    CollisionResult cr(true, Vector2D(0, 0), 0.0, Vector2D(0, 0));
    return cr;
}

CollisionResult detectCarVsCar(const RectangleOBB& carA, const RectangleOBB& carB) {
    CollisionResult cr(true, Vector2D(0, 0), 0.0, Vector2D(0, 0));
    return cr;
}

void resolveCarVsWall(Car& car, const CollisionResult& result) {
    return;
}

void resolveCarVsCar(Car& carA, Car& carB, const CollisionResult& result) {
    return;
}