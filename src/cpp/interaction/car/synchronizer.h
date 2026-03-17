#ifndef CAR_SYNCHRONIZER_H
#define CAR_SYNCHRONIZER_H

#include "../ipc/shared_memory.h"

class CarSynchronizer {
    bool isStepMode;
    SharedGameMemory& sgm;

    public:

    CarSynchronizer(bool isStepMode, SharedGameMemory& sgm) : isStepMode(isStepMode), sgm(sgm) {}

    bool isStepMode() {
        return isStepMode;
    }

    bool isActionReady() {
        if(!isStepMode) {
            return true; 
        }

        SharedGameData* data = sgm.getData();
        if(!data) {
            return false;
        }

        return data->action_ready;
    }

    void setActionReady(bool val) {
        if(!isStepMode) {
            return; 
        }

        SharedGameData* data = sgm.getData();
        if(!data) {
            return;
        }

        data->action_ready = val;
    }

    bool isResetFlagSet() {
        if(!isStepMode) {
            return false; 
        }

        SharedGameData* data = sgm.getData();
        if(!data) {
            return false;
        }

        return data->reset_flag;
    }

    void setResetFlag(bool val) {
        if(!isStepMode) {
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