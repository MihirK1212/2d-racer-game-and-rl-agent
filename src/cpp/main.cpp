#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <ctime>
#include <cmath>
#include <optional>
#include <string>

#include "./entity/car/car.h"
#include "./engine/vector.h"
#include "./engine/coordinates.h"
#include "./engine/collision/collision.h"
#include "./entity/curve/circular_curve.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FRAME_RATE 60

sf::RenderWindow createWindow()
{
    sf::RenderWindow window(
        sf::VideoMode({1000, 600}),
        "Car Driving Game");

    window.setFramerateLimit(FRAME_RATE);
    return window;
}

sf::RectangleShape createCarShape(Car *car, CoordinateTransform &coordTransform)
{
    sf::RectangleShape shape({static_cast<float>(coordTransform.metersToPixels(car->getWidth())),
                              static_cast<float>(coordTransform.metersToPixels(car->getHeight()))});

    shape.setFillColor(sf::Color::Red);

    shape.setOrigin({static_cast<float>(coordTransform.metersToPixels(car->getWidth()) / 2.0),
                     static_cast<float>(coordTransform.metersToPixels(car->getHeight()) / 2.0)});

    return shape;
}

void processEvents(sf::RenderWindow &window, Car *car)
{
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            window.close();

        if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Num9)
                car->printCarState();
        }
    }
}

void handleContinuousInput(Car *car)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        car->rotateAntiClockwise(3);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        car->rotateClockwise(3);

    car->setNoInputAcceleration();

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        car->accelerateForward();

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        car->accelerateBackward();
}

void updateGame(Car *car)
{
    car->move(1.0 / FRAME_RATE);
}

void drawDirectionArrow(sf::RenderWindow &window,
                        const Vector2D &screenPos,
                        const Vector2D &screenDir)
{
    const float arrowLength = 30.f;
    const float headLength = 8.f;
    const float headAngle = 0.45f;

    float cx = static_cast<float>(screenPos.x);
    float cy = static_cast<float>(screenPos.y);

    float tipX = cx + static_cast<float>(screenDir.x) * arrowLength;
    float tipY = cy + static_cast<float>(screenDir.y) * arrowLength;

    Vector2D back = (screenDir * -1.0).normalize();
    Vector2D leftWing = back.rotate(headAngle) * headLength;
    Vector2D rightWing = back.rotate(-headAngle) * headLength;

    sf::VertexArray arrow(sf::PrimitiveType::Lines, 6);

    arrow[0] = sf::Vertex{{cx, cy}, sf::Color::Yellow};
    arrow[1] = sf::Vertex{{tipX, tipY}, sf::Color::Yellow};

    arrow[2] = sf::Vertex{{tipX, tipY}, sf::Color::Yellow};
    arrow[3] = sf::Vertex{{tipX + (float)leftWing.x, tipY + (float)leftWing.y}, sf::Color::Yellow};

    arrow[4] = sf::Vertex{{tipX, tipY}, sf::Color::Yellow};
    arrow[5] = sf::Vertex{{tipX + (float)rightWing.x, tipY + (float)rightWing.y}, sf::Color::Yellow};

    window.draw(arrow);
}

void drawCurve(sf::RenderWindow &window,
    CircularCurve &curve,
    CoordinateTransform &coordTransform)
{
    std::vector<Curve2DPoint> points = curve.getCachedPoints();
    size_t numPoints = points.size();

    if (numPoints < 2)
    return;

    sf::VertexArray vertices(sf::PrimitiveType::LineStrip, numPoints);

    for (size_t i = 0; i < numPoints; ++i)
    {
        Vector2D screenPos = coordTransform.gameToScreenPoint(points[i].position);
        vertices[i].position =
        {
            static_cast<float>(screenPos.x),
            static_cast<float>(screenPos.y)
        };

        vertices[i].color = sf::Color::White; // optional
    }

    window.draw(vertices);
}

void render(sf::RenderWindow &window,
            Car *car,
            CoordinateTransform &coordTransform,
            sf::RectangleShape &carShape,
            CircularCurve &innerBorder,
            CircularCurve &outerBorder)
{
    Vector2D screenPos = coordTransform.gameToScreenPoint(car->getPosition());
    carShape.setPosition({static_cast<float>(screenPos.x),
                          static_cast<float>(screenPos.y)});

    Vector2D screenDir = coordTransform.gameToScreenDirection(car->getDirection());

    float angleDeg = static_cast<float>(
        std::atan2(screenDir.y, screenDir.x) * 180.0 / M_PI - 90.0);

    carShape.setRotation(sf::degrees(angleDeg));

    window.clear(sf::Color::Black);

    window.draw(carShape);

    drawDirectionArrow(window, screenPos, screenDir);

    drawCurve(window, innerBorder, coordTransform);
    drawCurve(window, outerBorder, coordTransform);

    window.display();
}

int main()
{
    sf::RenderWindow window = createWindow();

    CoordinateTransform coordTransform(1000, 600);

    Car *car = new Car(0, 0, 0.7, 1.5);

    CircularCurve innerBorder(20, 0, 0), outerBorder(27, 0, 0);
    innerBorder.generate(0, 360, 100);
    outerBorder.generate(0, 360, 100);

    sf::RectangleShape carShape = createCarShape(car, coordTransform);

    while (window.isOpen())
    {
        processEvents(window, car);

        handleContinuousInput(car);

        updateGame(car);

        render(window, car, coordTransform, carShape, innerBorder, outerBorder);
    }

    delete car;

    return 0;
}