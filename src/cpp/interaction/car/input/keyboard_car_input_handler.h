#ifndef KEYBOARD_CAR_INPUT_HANDLER_H
#define KEYBOARD_CAR_INPUT_HANDLER_H

#include <SFML/Graphics.hpp>
#include "./base_car_input_handler.h"

struct CarKeyBindings {
    sf::Keyboard::Key antiClockwise = sf::Keyboard::Key::Left;
    sf::Keyboard::Key clockwise     = sf::Keyboard::Key::Right;
    sf::Keyboard::Key forward       = sf::Keyboard::Key::Up;
    sf::Keyboard::Key backward      = sf::Keyboard::Key::Down;
};

class KeyboardCarInputHandler: public CarInputHandler {
    CarKeyBindings bindings;

    public:

    explicit KeyboardCarInputHandler(CarKeyBindings bindings = {})
        : bindings(bindings) {}

    void apply(Car& car) override {
        if (sf::Keyboard::isKeyPressed(bindings.antiClockwise))
            car.rotateAntiClockwise(3);

        if (sf::Keyboard::isKeyPressed(bindings.clockwise))
            car.rotateClockwise(3);

        car.setNoInputAcceleration();

        if (sf::Keyboard::isKeyPressed(bindings.forward))
            car.accelerateForward();

        if (sf::Keyboard::isKeyPressed(bindings.backward))
            car.accelerateBackward();
    }
};

#endif
