#ifndef SHM_GAME_STATE_EXPORTER_H
#define SHM_GAME_STATE_EXPORTER_H

#include "../../entity/car/car.h"
#include "../ipc/shared_memory.h"

class SHMGameStateExporter {
    SharedGameMemory& sgm;

    public:

    explicit SHMGameStateExporter(SharedGameMemory& sgm) : sgm(sgm) {}

    void exportState(const Car& car) {
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
