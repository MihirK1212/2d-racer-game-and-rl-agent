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

        verifyData();
    }

    void verifyData() {
        SharedGameData* data = sgm.getData();
        if (!data) {
            return;
        }
        std::cout << "Data is not null" << std::endl;
        std::cout << "pos_x: " << data->pos_x << std::endl;
        std::cout << "pos_y: " << data->pos_y << std::endl;
        std::cout << "speed: " << data->speed << std::endl;
        std::cout << "dir_x: " << data->dir_x << std::endl;
        std::cout << "dir_y: " << data->dir_y << std::endl;
        std::cout << "tangential_accel: " << data->tangential_accel << std::endl;
        std::cout << "state_ready: " << data->state_ready << std::endl;
    }
};

#endif
