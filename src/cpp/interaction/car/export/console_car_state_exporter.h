#ifndef CONSOLE_CAR_STATE_EXPORTER_H
#define CONSOLE_CAR_STATE_EXPORTER_H

#include "./base_car_state_exporter.h"

class ConsoleCarStateExporter: public CarStateExporter {
    public:

    void exportCarState(const Car& car) override {
        car.printCarState();
    }
};

#endif
