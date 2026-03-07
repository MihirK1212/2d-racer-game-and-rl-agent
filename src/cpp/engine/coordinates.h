#ifndef COORDINATES_H
#define COORDINATES_H

#include "../engine/vector.h"

/*
 * Transforms between two 2D coordinate systems:
 *
 * Game  (internal state): origin at CENTER of screen, x-right, y-UP
 * Screen (SFML render):   origin at TOP-LEFT,          x-right, y-DOWN
 *
 * Basis change (y-flip):
 *   e_screen_x = ( 1,  0 )
 *   e_screen_y = ( 0, -1 )
 *
 * Point transform (basis change + origin translation):
 *   screen_x =  game_x + screenWidth  / 2
 *   screen_y = -game_y + screenHeight / 2
 *
 * Free vector transform (basis change only, no translation):
 *   screen_vx =  game_vx
 *   screen_vy = -game_vy
 */
struct CoordinateTransform {
    double screenWidth;
    double screenHeight;

    CoordinateTransform(double screenWidth, double screenHeight)
        : screenWidth(screenWidth), screenHeight(screenHeight) {}

    Vector2D gameToScreenPoint(const Vector2D& p) const {
        return {p.x + screenWidth / 2.0, -p.y + screenHeight / 2.0};
    }

    Vector2D screenToGamePoint(const Vector2D& p) const {
        return {p.x - screenWidth / 2.0, -(p.y - screenHeight / 2.0)};
    }

    Vector2D gameToScreenVector(const Vector2D& v) const {
        return {v.x, -v.y};
    }

    Vector2D screenToGameVector(const Vector2D& v) const {
        return {v.x, -v.y};
    }
};

#endif
