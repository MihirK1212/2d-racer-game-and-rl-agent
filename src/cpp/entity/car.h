#ifndef CAR_H
#define CAR_H

#include "engine/vector.h"

class Car {
    int x; // position of the center of the car on the x-axis
    int y; // position of the center of the car on the y-axis
    int width; // width of the car
    int height; // height of the car
    Vector2D velocity; // velocity of the car
    Vector2D acceleration; // acceleration of the car
};

#endif