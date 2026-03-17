#ifndef SHM_CAR_INPUT_HANDLER_H
#define SHM_CAR_INPUT_HANDLER_H

#include "./base_car_input_handler.h"
#include "../../ipc/shared_memory.h"

class SHMCarInputHandler: public CarInputHandler {
    SharedGameMemory& sgm;

    public:

    explicit SHMCarInputHandler(SharedGameMemory& sgm) : sgm(sgm) {}

    void apply(Car& car) override {
        SharedGameData* data = sgm.getData();
        if(!data) {return;}

        if (data->input_left)
            car.rotateAntiClockwise(3);
        if (data->input_right)
            car.rotateClockwise(3);

        car.setNoInputAcceleration();

        if (data->input_up)
            car.accelerateForward();
        if (data->input_down)
            car.accelerateBackward();
    }
};

#endif
