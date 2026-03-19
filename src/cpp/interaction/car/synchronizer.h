#ifndef CAR_SYNCHRONIZER_H
#define CAR_SYNCHRONIZER_H

#include "../ipc/shared_memory.h"

class CarSynchronizer {
    bool rlMode;
    bool stepMode;
    SharedGameMemory& sgm;

    public:

    CarSynchronizer(bool rlMode, bool stepMode, SharedGameMemory& sgm) : rlMode(rlMode), stepMode(stepMode), sgm(sgm) {}

    bool isStepMode() {
        return stepMode;
    }

    bool isRlMode() {
        return rlMode;
    }

    bool isActionReady() {
        if(!isRlMode()) {
            // in non-rl mode, action is always ready, as it is controlled by the user
            return true; 
        }

        SharedGameData* data = sgm.getData();
        if(!data) {
            return false;
        }

        return data->action_ready;
    }

    void setActionReady(bool val) {
        if(!isRlMode()) {
            // in non-rl mode, action is always ready, as it is controlled by the user
            return; 
        }

        SharedGameData* data = sgm.getData();
        if(!data) {
            return;
        }

        data->action_ready = val;
    }

    bool isResetFlagSet() {
        if(!isRlMode()) {
            // in non-rl mode, reset flag is always unset, as it is controlled by the user
            return false; 
        }

        SharedGameData* data = sgm.getData();
        if(!data) {
            return false;
        }

        return data->reset_flag;
    }

    void setResetFlag(bool val) {
        if(!isRlMode()) {
            // in non-rl mode, reset flag is always unset, as it is controlled by the user
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