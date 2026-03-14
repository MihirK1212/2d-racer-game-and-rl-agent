#ifndef BASE_CAR_STATE_EXPORTER_H
#define BASE_CAR_STATE_EXPORTER_H

#include "../../../entity/car/car.h"

class CarStateExporter {
    public:

    virtual void exportCarState(const Car& car) = 0;
    virtual ~CarStateExporter() = default;
};

#endif
