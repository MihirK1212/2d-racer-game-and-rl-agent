#include "engine/vector.h"


class Car {
    int x; // position of the center of the car on the x-axis
    int y; // position of the center of the car on the y-axis
    int width; // width of the car
    int height; // height of the car
    Vector2D velocity; // velocity of the car
    Vector2D acceleration; // acceleration of the car

    public:

    
    Car(int x_val, int y_val, int width_val, int height_val) : x(x_val), y(y_val), width(width_val), height(height_val) {}

    // double getDirection() {

    double getSpeed() {
        return velocity.magnitude();
    }
    
};