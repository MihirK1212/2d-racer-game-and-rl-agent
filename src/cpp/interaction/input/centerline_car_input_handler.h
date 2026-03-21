#ifndef CENTERLINE_CAR_INPUT_HANDLER_H
#define CENTERLINE_CAR_INPUT_HANDLER_H

#include "./base_car_input_handler.h"
#include "../../entity/curve/curve.h"
#include "../../engine/vector.h"

class CenterlineCarInputHandler : public CarInputHandler {
    ParametricCurve2D* centerline;
    static constexpr double STEER_ANGLE = 3.0;

public:
    explicit CenterlineCarInputHandler(ParametricCurve2D* centerline)
        : centerline(centerline) {}

    void apply(Car& car) override {
        ClosestPointResult closest = centerline->closestPointTo(car.getPosition());
        Vector2D desiredDir = centerline->tangentAt(closest.theta);

        Vector2D carDir = car.getDirection();
        double cross = Vector2D::cross(carDir, desiredDir);

        if (cross > 0.01)
            car.rotateAntiClockwise(STEER_ANGLE);
        else if (cross < -0.01)
            car.rotateClockwise(STEER_ANGLE);

        car.setNoInputAcceleration();
        car.accelerateForward();
    }
};

#endif
