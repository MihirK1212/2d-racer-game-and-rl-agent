#include <algorithm>
#include <cmath>
#include <iostream>

#include "car.h"
#include "../engine/vector.h"

Car::Car(
    int x_val, int y_val, int width_val, int height_val, 
    double maxSpeedForward_val,
    double maxSpeedBackward_val,
    double driveAcceleration_val,
    double brakeAcceleration_val,
    double maxTangentialAcceleration_val,
    double friction_val
) {
    this->x = x_val;
    this->y = y_val;
    this->width = width_val;
    this->height = height_val;
    this->maxSpeedForward = maxSpeedForward_val;
    this->maxSpeedBackward = maxSpeedBackward_val;
    this->driveAcceleration = driveAcceleration_val;
    this->brakeAcceleration = brakeAcceleration_val;
    this->maxTangentialAcceleration = maxTangentialAcceleration_val;
    this->speed = 0;
    this->direction = Vector2D(1, 0);
    this->tangentialAcceleration = 0;
    this->friction = friction_val;
}

double Car::getX()
{
    return x;
}

double Car::getY()
{
    return y;
}

double Car::getWidth()
{
    return width;
}

double Car::getHeight()
{
    return height;
}

Vector2D Car::getVelocity()
{
    return direction * speed;
}

Vector2D Car::getDirection()
{
    return direction;
}

void Car::move(double dt)
{   
    bool initiallyZero = (std::abs(speed) < 0.01);

    bool signBefore = speed >= 0;
    speed += tangentialAcceleration * dt;
    bool signAfter = speed >= 0; 

    if(!initiallyZero && signBefore != signAfter) {
        // if the sign of the speed changes, set the speed to 0
        speed = 0;
    }

    speed = std::clamp(speed, -maxSpeedBackward, maxSpeedForward);

    x += speed * direction.x * dt;
    y += speed * direction.y * dt;
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


void Car::rotateAntiClockwise(double angleDegrees) {
    if(std::abs(speed) > 0.01) {
        double angleRadians = angleDegrees * M_PI / 180.0;
        direction = direction.rotate(angleRadians).normalize();
    }
}

void Car::rotateClockwise(double angleDegrees) {
    if(std::abs(speed) > 0.01) {
        double angleRadians = angleDegrees * M_PI / 180.0;
        direction = direction.rotate(-angleRadians).normalize();
    }
}

void Car::printCarState()
{
    std::cout << "--------------------------------" << "\n";
    std::cout << "Car state: " << "\n";
    std::cout << "Position: (" << x << ", " << y << ")" << "\n";
    std::cout << "Speed: " << speed << "\n";
    std::cout << "Direction: (" << direction.x << ", " << direction.y << ")" << "\n";
    std::cout << "Tangential Acceleration: " << tangentialAcceleration << "\n";
    std::cout << "--------------------------------" << "\n";
}