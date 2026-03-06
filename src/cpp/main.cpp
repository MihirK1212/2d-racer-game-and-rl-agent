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

    sf::RectangleShape carShape({car->width, car->height});
    carShape.setPosition(car->x, car->y);
    carShape.setFillColor(sf::Color::Red);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }


        window.clear(sf::Color::Black);

        window.draw(carShape);

        window.display();
    }

   
    return 0;
}