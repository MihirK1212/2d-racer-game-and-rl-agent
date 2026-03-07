#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <ctime>
#include <optional>
#include <string>
#include "./entity/car.h"
#include "./engine/vector.h"

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

            if (const auto* keyPressed =
                event->getIf<sf::Event::KeyPressed>())
            {
                const auto key = keyPressed->code;

                // Print car state when 9 is pressed
                if ( key == sf::Keyboard::Key::Num9)
                {
                    car->printCarState();
                }
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

        // Direction arrow
        const float arrowLength = 30.f;
        const float headLength = 8.f;
        const float headAngle = 0.45f;

        Vector2D dir = car->getDirection();
        float cx = static_cast<float>(car->getX() + car->getWidth() / 2.0);
        float cy = static_cast<float>(car->getY() + car->getHeight() / 2.0);
        float tipX = cx + static_cast<float>(dir.x) * arrowLength;
        float tipY = cy + static_cast<float>(dir.y) * arrowLength;

        Vector2D back = (dir * -1.0).normalize();
        Vector2D leftWing = back.rotate(headAngle) * headLength;
        Vector2D rightWing = back.rotate(-headAngle) * headLength;

        sf::VertexArray arrow(sf::PrimitiveType::Lines, 6);
        arrow[0] = sf::Vertex{{cx, cy}, sf::Color::Yellow};
        arrow[1] = sf::Vertex{{tipX, tipY}, sf::Color::Yellow};
        arrow[2] = sf::Vertex{{tipX, tipY}, sf::Color::Yellow};
        arrow[3] = sf::Vertex{{tipX + static_cast<float>(leftWing.x), tipY + static_cast<float>(leftWing.y)}, sf::Color::Yellow};
        arrow[4] = sf::Vertex{{tipX, tipY}, sf::Color::Yellow};
        arrow[5] = sf::Vertex{{tipX + static_cast<float>(rightWing.x), tipY + static_cast<float>(rightWing.y)}, sf::Color::Yellow};

        window.draw(arrow);

        window.display();
    }

   
    return 0;
}