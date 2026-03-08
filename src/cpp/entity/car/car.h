#ifndef CAR_H
#define CAR_H

#include "../engine/vector.h"
#include "../engine/rectangle_obb.h"

class Car {
    // units are in meters and seconds

    Vector2D position; // position of the center of the car
    double width; // width of the car (meters)
    double height; // height of the car (meters)

    double speed; // speed of the car. +ve means moving in forward direction, -ve means moving in backward direction
    double tangentialAcceleration; // tangential acceleration of the car
    
    double maxSpeedForward; // maximum speed of the car when moving forward
    double maxSpeedBackward; // maximum speed of the car when moving backward

    double driveAcceleration; // constant acceleration of the car when throttle is applied
    double brakeAcceleration; // constant acceleration of the car when brake is applied

    double maxTangentialAcceleration; // maximum absolute tangential acceleration of the car (in either direction)

    double friction;

    double steeringSpeedFactor; // k in the model: effectiveTurnRate = angle / (1 + k * |speed|)

    Vector2D direction{}; // forward direction of the car

    public:

    Car(
        double x, double y, double width, double height, 
        double maxSpeedForward = 20, double maxSpeedBackward = 10, 
        double driveAcceleration = 2.5, double brakeAcceleration = 7.5,
        double maxTangentialAcceleration = 10,
        double friction = 0.5,
        double steeringSpeedFactor = 0.1
    );

    // Getters
    Vector2D getPosition() const;
    double getWidth() const;
    double getHeight() const;
    double getSpeed() const;
    double getTangentialAcceleration() const;
    double getMaxSpeedForward() const;
    double getMaxSpeedBackward() const;
    double getDriveAcceleration() const;
    double getBrakeAcceleration() const;
    double getMaxTangentialAcceleration() const;
    double getFriction() const;
    double getSteeringSpeedFactor() const;
    Vector2D getVelocity() const; // velocity vector of the car
    Vector2D getDirection() const; // forward direction of the car
    RectangleOBB getOBB() const; // get the oriented bounding box of the car

    // Setters (only for properties that can change after construction)
    void setPosition(Vector2D pos);
    void setSpeed(double speed);
    void setTangentialAcceleration(double tangentialAcceleration);
    void setDirection(Vector2D direction);

    void move(double dt); // move the car for the given time step

    void setNoInputAcceleration(); // reset the acceleration of the car when neither forward nor backward acceleration is applied
    void accelerateForward(); // accelerate car in forward direction
    void accelerateBackward(); // accelerate car in backward direction

    void rotateAntiClockwise(double angleDegrees); // rotate the car anti-clockwise by the given angle (in degrees)
    void rotateClockwise(double angleDegrees); // rotate the car clockwise by the given angle (in degrees)

    void printCarState();
};

#endif
