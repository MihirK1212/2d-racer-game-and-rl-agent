#ifndef CAR_SYNCHRONIZER_H
#define CAR_SYNCHRONIZER_H

#include "../ipc/shared_memory.h"

class CarSynchronizer {
    bool stepMode;
    SharedGameMemory& sgm;

    public:

    CarSynchronizer(bool stepMode, SharedGameMemory& sgm) : stepMode(stepMode), sgm(sgm) {}

    bool isStepMode() {
        return stepMode;
    }

    bool isActionReady() {
        if(!isStepMode()) {
            return true; 
        }

        SharedGameData* data = sgm.getData();
        if(!data) {
            return false;
        }

        return data->action_ready;
    }

    void setActionReady(bool val) {
        if(!isStepMode()) {
            return; 
        }

        SharedGameData* data = sgm.getData();
        if(!data) {
            return;
        }

        data->action_ready = val;
    }

    bool isResetFlagSet() {
        if(!isStepMode()) {
            return false; 
        }

        SharedGameData* data = sgm.getData();
        if(!data) {
            return false;
        }

        return data->reset_flag;
    }

    void setResetFlag(bool val) {
        if(!isStepMode()) {
            return; 
        }

        SharedGameData* data = sgm.getData();
        if(!data) {
            return;
        }

        data->reset_flag = val;
    }
};

#endif