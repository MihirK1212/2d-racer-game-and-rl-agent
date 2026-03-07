#ifndef COORDINATES_H
#define COORDINATES_H

#include "../engine/vector.h"

/*
 * Two-stage transform: Physics World (meters) → Render Space (pixels)
 *
 * Stage 1 – Unit conversion:
 *   pixels = meters × PIXELS_PER_METER
 *
 * Stage 2 – Coordinate-system change:
 *   Game  (internal state): origin at CENTER of screen, x-right, y-UP
 *   Screen (SFML render):   origin at TOP-LEFT,          x-right, y-DOWN
 *
 * Combined point transform:
 *   screen_x =  game_x * PPM + screenWidth  / 2
 *   screen_y = -game_y * PPM + screenHeight / 2
 *
 * Three vector flavours:
 *   Displacement (meters → pixels): applies scale + y-flip
 *   Direction    (unit → unit):     applies y-flip only
 */
struct CoordinateTransform {
    static constexpr double PIXELS_PER_METER = 10.0;

    double screenWidth;
    double screenHeight;

    CoordinateTransform(double screenWidth, double screenHeight)
        : screenWidth(screenWidth), screenHeight(screenHeight) {}

    // --- Points (position in meters ↔ position in pixels) ---

    Vector2D gameToScreenPoint(const Vector2D& p) const {
        return {
             p.x * PIXELS_PER_METER + screenWidth  / 2.0,
            -p.y * PIXELS_PER_METER + screenHeight / 2.0
        };
    }

    Vector2D screenToGamePoint(const Vector2D& p) const {
        return {
             (p.x - screenWidth  / 2.0) / PIXELS_PER_METER,
            -(p.y - screenHeight / 2.0) / PIXELS_PER_METER
        };
    }

    // --- Displacement vectors (meters → pixels, e.g. velocity, offset) ---

    Vector2D gameToScreenVector(const Vector2D& v) const {
        return {v.x * PIXELS_PER_METER, -v.y * PIXELS_PER_METER};
    }

    Vector2D screenToGameVector(const Vector2D& v) const {
        return {v.x / PIXELS_PER_METER, -v.y / PIXELS_PER_METER};
    }

    // --- Direction vectors (unit vector, y-flip only, no scaling) ---

    Vector2D gameToScreenDirection(const Vector2D& d) const {
        return {d.x, -d.y};
    }

    Vector2D screenToGameDirection(const Vector2D& d) const {
        return {d.x, -d.y};
    }

    // --- Scalar helpers ---

    double metersToPixels(double meters) const {
        return meters * PIXELS_PER_METER;
    }

    double pixelsToMeters(double pixels) const {
        return pixels / PIXELS_PER_METER;
    }
};

#endif
