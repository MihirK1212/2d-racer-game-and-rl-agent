#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <ctime>
#include <optional>
#include <string>
#include "./entity/car.h"

int main()
{
    // Create large window
    sf::RenderWindow window(
        sf::VideoMode({1000, 600}),
        "Car Driving Game");

    window.setFramerateLimit(60);

    Car* car = new Car(500, 300, 30, 30);

    sf::RectangleShape carShape({car->getWidth(), car->getHeight()});
    carShape.setFillColor(sf::Color::Red);

    while (window.isOpen())
    {   
        /*
            1. Process Events (things that happen once)
            2. Handle Continuous Input
            3. Update Game State
            4. Render
        */

        // 1. Process Events
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        // 2. Handle Continuous Input
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            car->rotateAntiClockwise(1);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            car->rotateClockwise(1);

        car->setNoInputAcceleration();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            car->accelerateForward();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            car->accelerateBackward();

        // 3. Update Game State
        car->move(1.0 / 60.0);
        carShape.setPosition({car->getX(), car->getY()});

        // 4. Render
        window.clear(sf::Color::Black);

        window.draw(carShape);

        window.display();
    }

   
    return 0;
}