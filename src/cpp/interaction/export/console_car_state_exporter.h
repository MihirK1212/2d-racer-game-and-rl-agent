#ifndef CONSOLE_CAR_STATE_EXPORTER_H
#define CONSOLE_CAR_STATE_EXPORTER_H

#include "../../entity/car/car.h"

class ConsoleCarStateExporter {
    public:

    void exportCarState(const Car& car) {
        car.printCarState();
    }
};

#endif
