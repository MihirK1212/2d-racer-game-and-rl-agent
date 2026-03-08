#include <algorithm>
#include <cmath>
#include <iostream>
#include <thread>
#include <chrono>

#include "car.h"
#include "../engine/vector.h"

#define M_PI 3.14159265358979323846


Car::Car(
    double x_val, double y_val, double width_val, double height_val, 
    double maxSpeedForward_val,
    double maxSpeedBackward_val,
    double driveAcceleration_val,
    double brakeAcceleration_val,
    double maxTangentialAcceleration_val,
    double friction_val,
    double steeringSpeedFactor_val
) {
    this->position = Vector2D(x_val, y_val);
    this->width = width_val;
    this->height = height_val;
    this->maxSpeedForward = maxSpeedForward_val;
    this->maxSpeedBackward = maxSpeedBackward_val;
    this->driveAcceleration = driveAcceleration_val;
    this->brakeAcceleration = brakeAcceleration_val;
    this->maxTangentialAcceleration = maxTangentialAcceleration_val;
    this->speed = 0;
    this->direction = Vector2D(0, 1);
    this->tangentialAcceleration = 0;
    this->friction = friction_val;
    this->steeringSpeedFactor = steeringSpeedFactor_val;
}

Vector2D Car::getPosition() const
{
    return position;
}

double Car::getWidth() const
{
    return width;
}

double Car::getHeight() const
{
    return height;
}

double Car::getSpeed() const
{
    return speed;
}

double Car::getTangentialAcceleration() const
{
    return tangentialAcceleration;
}

double Car::getMaxSpeedForward() const
{
    return maxSpeedForward;
}

double Car::getMaxSpeedBackward() const
{
    return maxSpeedBackward;
}

double Car::getDriveAcceleration() const
{
    return driveAcceleration;
}

double Car::getBrakeAcceleration() const
{
    return brakeAcceleration;
}

double Car::getMaxTangentialAcceleration() const
{
    return maxTangentialAcceleration;
}

double Car::getFriction() const
{
    return friction;
}

double Car::getSteeringSpeedFactor() const
{
    return steeringSpeedFactor;
}

Vector2D Car::getVelocity() const
{
    return direction * speed;
}

Vector2D Car::getDirection() const
{
    return direction;
}

void Car::setPosition(Vector2D pos)
{
    this->position = pos;
}

void Car::setSpeed(double speed)
{
    this->speed = speed;
}

void Car::setTangentialAcceleration(double tangentialAcceleration)
{
    this->tangentialAcceleration = tangentialAcceleration;
}

void Car::setDirection(Vector2D direction)
{
    this->direction = direction;
}

void Car::move(double dt)
{   
    bool initiallySpeedZero = (std::abs(speed) < 0.01);
    bool initiallyAccelerationZero = (std::abs(tangentialAcceleration) < 0.01);

    if(initiallySpeedZero && initiallyAccelerationZero) {
        // car is stationary and no acceleration, no need to move the car
        return;
    }

    bool signBefore = speed >= 0;
    speed += tangentialAcceleration * dt;
    bool signAfter = speed >= 0; 

    if(!initiallySpeedZero && signBefore != signAfter) {
        // if the sign of the speed changes, set the speed to 0 (simulate the car pausing briefly)
        speed = 0;
    }

    speed = std::clamp(speed, -maxSpeedBackward, maxSpeedForward);

    position.x += speed * direction.x * dt;
    position.y += speed * direction.y * dt;
}


void Car::setNoInputAcceleration()
{
    if((std::abs(speed) < 0.01)) {
        // if car is already stationary, set acceleration to 0 
        tangentialAcceleration = 0;
    }
    else {
        // if car is moving, apply friction
        // sign(speed): -1 for negative, 0 for zero, +1 for positive
        const double speedSign = (speed > 0.0) - (speed < 0.0);
        tangentialAcceleration = -friction * speedSign;
    }
}

void Car::accelerateForward()
{
    if(speed >= 0) { // moving forward, want to accelerate the car
        // drive accelerate if currently moving forward
        tangentialAcceleration = driveAcceleration;
    }
    else if(speed < 0) { // moving backward, want to stop the car
        // break decelerate if currently moving backward
        tangentialAcceleration = brakeAcceleration;
    }

    tangentialAcceleration = std::clamp(tangentialAcceleration, -maxTangentialAcceleration, maxTangentialAcceleration);
}


void Car::accelerateBackward()
{
    if(speed <= 0) { // moving backward, want to accelerate the car
        // drive accelerate if currently moving backward
        tangentialAcceleration = -driveAcceleration;
    }
    else if(speed > 0) { // moving forward, want to stop the car
        // brake decelerate if currently moving forward
        tangentialAcceleration = -brakeAcceleration;
    }

    tangentialAcceleration = std::clamp(tangentialAcceleration, -maxTangentialAcceleration, maxTangentialAcceleration);
}


/*
The car's turning behavior is modeled using a simple equation:
effectiveTurnRate = angle / (1 + k * |speed|)
where:
angle is the angle by which the car is turned (in degrees)
k is the steering speed factor (default 0.1)
speed is the car's speed (in meters per second)
effectiveTurnRate is the effective turn rate of the car (in degrees per second)

Behavior with k = 0.1:
Speed (m/s)	    Steering fraction (percentage of full angle per frame)
0	            100% (full 3°/frame)
5	            67% (2°/frame)
10	            50% (1.5°/frame)
20 (max)	    33% (1°/frame)
*/

void Car::rotateAntiClockwise(double angleDegrees) {
    if(std::abs(speed) > 0.01) {
        double effectiveAngle = angleDegrees / (1.0 + steeringSpeedFactor * std::abs(speed));
        double angleRadians = effectiveAngle * M_PI / 180.0;
        direction = direction.rotate(angleRadians).normalize();
    }
}

void Car::rotateClockwise(double angleDegrees) {
    if(std::abs(speed) > 0.01) {
        double effectiveAngle = angleDegrees / (1.0 + steeringSpeedFactor * std::abs(speed));
        double angleRadians = effectiveAngle * M_PI / 180.0;
        direction = direction.rotate(-angleRadians).normalize();
    }
}

void Car::printCarState()
{
    std::cout << "--------------------------------" << "\n";
    std::cout << "Car state: " << "\n";
    std::cout << "Position: (" << position.x << ", " << position.y << ")" << "\n";
    std::cout << "Speed: " << speed << "\n";
    std::cout << "Direction: (" << direction.x << ", " << direction.y << ")" << "\n";
    std::cout << "Tangential Acceleration: " << tangentialAcceleration << "\n";
    std::cout << "--------------------------------" << "\n";
}
