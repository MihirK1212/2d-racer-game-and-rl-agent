#ifndef BASE_CAR_INPUT_HANDLER_H
#define BASE_CAR_INPUT_HANDLER_H

#include "../../entity/car/car.h"

class CarInputHandler {
    public:
    
    virtual void apply(Car& car) = 0;
    virtual ~CarInputHandler() = default;
};

#endif
