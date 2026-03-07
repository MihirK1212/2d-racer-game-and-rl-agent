#ifndef CAR_H
#define CAR_H

#include "../engine/vector.h"

class Car {
    double x; // position of the center of the car on the x-axis
    double y; // position of the center of the car on the y-axis
    int width; // width of the car
    int height; // height of the car

    double speed; // speed of the car. +ve means moving in forward direction, -ve means moving in backward direction
    double tangentialAcceleration; // tangential acceleration of the car
    
    double maxSpeedForward; // maximum speed of the car when moving forward
    double maxSpeedBackward; // maximum speed of the car when moving backward

    double driveAcceleration; // constant acceleration of the car when throttle is applied
    double brakeAcceleration; // constant acceleration of the car when brake is applied

    double maxTangentialAcceleration; // maximum absolute tangential acceleration of the car (in either direction)

    double friction;

    Vector2D direction{}; // forward direction of the car

    public:

    Car(
        int x, int y, int width, int height, 
        double maxSpeedForward = 100, double maxSpeedBackward = 50, 
        double driveAcceleration = 10, double brakeAcceleration = 5,
        double maxTangentialAcceleration = 10,
        double friction = 0.1
    );

    double getX();
    double getY();
    double getWidth();
    double getHeight();
    Vector2D getVelocity(); // get the velocity of the car
    Vector2D getDirection(); // get the forward direction of the car

    void move(double dt); // move the car for the given time step

    void setNoInputAcceleration(); // reset the acceleration of the car when neither forward nor backward acceleration is applied
    void accelerateForward(); // accelerate car in forward direction
    void accelerateBackward(); // accelerate car in backward direction

    void rotateAntiClockwise(double angleDegrees); // rotate the car anti-clockwise by the given angle (in degrees)
    void rotateClockwise(double angleDegrees); // rotate the car clockwise by the given angle (in degrees)


    void printCarState();
};

#endif