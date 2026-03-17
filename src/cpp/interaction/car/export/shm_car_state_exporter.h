#ifndef SHM_CAR_STATE_EXPORTER_H
#define SHM_CAR_STATE_EXPORTER_H

#include "./base_car_state_exporter.h"
#include "../../ipc/shared_memory.h"

class SHMCarStateExporter: public CarStateExporter {
    SharedGameMemory& sgm;

    public:

    explicit SHMCarStateExporter(SharedGameMemory& sgm) : sgm(sgm) {}

    void exportCarState(const Car& car) override {
        SharedGameData* data = sgm.getData();

        if (!data) {
            return;
        }
        
        Vector2D pos = car.getPosition();
        Vector2D dir = car.getDirection();
        
        data->pos_x = pos.x;
        data->pos_y = pos.y;
        data->speed = car.getSpeed();
        data->dir_x = dir.x;
        data->dir_y = dir.y;
        data->tangential_accel = car.getTangentialAcceleration();
        data->state_ready = 1;
    }
};

#endif
