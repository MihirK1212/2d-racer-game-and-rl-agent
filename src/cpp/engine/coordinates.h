#ifndef COORDINATES_H
#define COORDINATES_H

#include "vector.h"

/*
 * Transforms between two 2D coordinate systems that share the same x-axis
 * but have opposite y-axes.
 *
 * Game  (internal state): x-right, y-UP    (standard math)
 * Screen (SFML render):   x-right, y-DOWN
 *
 * The basis vectors of the screen frame expressed in game frame are:
 *   e_screen_x = ( 1,  0 )   (same)
 *   e_screen_y = ( 0, -1 )   (flipped)
 *
 * For points, an additional translation accounts for the vertical offset
 * so that game-origin bottom-left maps to screen-origin top-left:
 *   screen_point = (game_x,  screenHeight - game_y)
 *
 * For free vectors (directions, velocities, etc.) only the basis change
 * applies — no translation:
 *   screen_vec   = (game_vx, -game_vy)
 *
 * Both transforms are self-inverse, but named pairs are provided for clarity.
 */
struct CoordinateTransform {
    double screenHeight;

    explicit CoordinateTransform(double screenHeight)
        : screenHeight(screenHeight) {}

    Vector2D gameToScreenPoint(const Vector2D& p) const {
        return {p.x, screenHeight - p.y};
    }

    Vector2D screenToGamePoint(const Vector2D& p) const {
        return {p.x, screenHeight - p.y};
    }

    Vector2D gameToScreenVector(const Vector2D& v) const {
        return {v.x, -v.y};
    }

    Vector2D screenToGameVector(const Vector2D& v) const {
        return {v.x, -v.y};
    }
};

#endif
