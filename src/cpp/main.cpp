#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <ctime>
#include <cmath>
#include <optional>
#include <string>
#include <iostream>
#include <memory>

#include "./entity/car/car.h"
#include "./engine/vector.h"
#include "./engine/coordinates.h"
#include "./engine/collision/collision.h"
#include "./entity/curve/circular_curve.h"
#include "./interaction/car/input/keyboard_car_input_handler.h"
#include "./interaction/car/input/shm_car_input_handler.h"
#include "./interaction/car/export/console_car_state_exporter.h"
#include "./interaction/car/export/shm_car_state_exporter.h"
#include "./interaction/ipc/shared_memory.h"
#include "./interaction/car/synchronizer.h"

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

sf::RectangleShape createCarShape(const Car &car, CoordinateTransform &coordTransform)
{
    sf::RectangleShape shape({static_cast<float>(coordTransform.metersToPixels(car.getWidth())),
                              static_cast<float>(coordTransform.metersToPixels(car.getHeight()))});

    shape.setFillColor(sf::Color::Red);

    shape.setOrigin({static_cast<float>(coordTransform.metersToPixels(car.getWidth()) / 2.0),
                     static_cast<float>(coordTransform.metersToPixels(car.getHeight()) / 2.0)});

    return shape;
}


void updateGame(Car &car)
{
    car.move(1.0 / FRAME_RATE);
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
               CircularCurve *curve,
               CoordinateTransform &coordTransform)
{
    std::vector<Curve2DPoint> points = curve->getCachedPoints();
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
                static_cast<float>(screenPos.y)};

        vertices[i].color = sf::Color::White;
    }

    window.draw(vertices);
}

void handleCollisions(const std::vector<std::unique_ptr<Car>> &cars,
                      CircularCurve *innerBorder,
                      CircularCurve *outerBorder)
{
    size_t numCars = cars.size();

    for (size_t i = 0; i < numCars; i++)
    {
        Car *car = cars[i].get();

        CollisionResult resultInner = detectCarVsCurve(car, innerBorder);
        if (resultInner.collided)
            resolveCarVsCurve(car, innerBorder, resultInner);

        CollisionResult resultOuter = detectCarVsCurve(car, outerBorder);
        if (resultOuter.collided)
            resolveCarVsCurve(car, outerBorder, resultOuter);
    }

    for (size_t i = 0; i < numCars; i++)
    {
        Car *car1 = cars[i].get();

        for (size_t j = i + 1; j < numCars; j++)
        {
            Car *car2 = cars[j].get();

            CollisionResult result = detectCarVsCar(car1, car2);
            if (result.collided)
                resolveCarVsCar(car1, car2, result);
        }
    }
}

void render(sf::RenderWindow &window,
            const std::vector<std::unique_ptr<Car>> &cars,
            std::vector<sf::RectangleShape> &carShapes,
            CoordinateTransform &coordTransform,
            CircularCurve *innerBorder,
            CircularCurve *outerBorder)
{
    window.clear(sf::Color::Black);

    for (size_t i = 0; i < cars.size(); i++)
    {
        Car *car = cars[i].get();
        sf::RectangleShape &shape = carShapes[i];

        Vector2D screenPos = coordTransform.gameToScreenPoint(car->getPosition());

        shape.setPosition({static_cast<float>(screenPos.x),
                           static_cast<float>(screenPos.y)});

        Vector2D screenDir = coordTransform.gameToScreenDirection(car->getDirection());

        float angleDeg = static_cast<float>(
            std::atan2(screenDir.y, screenDir.x) * 180.0 / M_PI - 90.0);

        shape.setRotation(sf::degrees(angleDeg));

        window.draw(shape);

        drawDirectionArrow(window, screenPos, screenDir);
    }

    drawCurve(window, innerBorder, coordTransform);
    drawCurve(window, outerBorder, coordTransform);

    window.display();
}

int main()
{
    sf::RenderWindow window = createWindow();

    std::vector<std::unique_ptr<Car>> cars;
    cars.push_back(std::make_unique<Car>(22.5, 0, 0.7, 1.5));
    cars.push_back(std::make_unique<Car>(25.5, 0, 0.7, 1.5));

    SharedGameMemory shm;

    auto inputHandler1 = std::make_unique<KeyboardCarInputHandler>();
    auto inputHandler2 = std::make_unique<SHMCarInputHandler>(shm);

    std::vector<std::unique_ptr<CarStateExporter>> outputHandlers;
    outputHandlers.push_back(std::make_unique<ConsoleCarStateExporter>());
    outputHandlers.push_back(std::make_unique<SHMCarStateExporter>(shm));

    auto synchronizer = std::make_unique<CarSynchronizer>(true, shm);

    CoordinateTransform coordTransform(1000, 600);

    auto innerBorder = std::make_unique<CircularCurve>(20, 0, 0);
    auto outerBorder = std::make_unique<CircularCurve>(28, 0, 0);

    innerBorder->generate(0, 360, 100);
    outerBorder->generate(0, 360, 100);

    std::vector<sf::RectangleShape> carShapes;

    for (const auto &car : cars)
        carShapes.push_back(createCarShape(*car, coordTransform));

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        if(synchronizer->isStepMode()) {
            for (const auto &outputHandler : outputHandlers) {
                outputHandler->exportCarState(*cars[1]);
            }

            while(!synchronizer->isActionReady()){
                for(int i=0; i<10; i++){
                    std::cout<<"Waiting for action ready"<<std::endl;
                }
            }
            synchronizer->setActionReady(false);

            if(synchronizer->isResetFlagSet()){
                // reset episode
                synchronizer->setResetFlag(false);
            }
        }

        inputHandler1->apply(*cars[0]);
        inputHandler2->apply(*cars[1]);

        updateGame(*cars[0]);
        updateGame(*cars[1]);

        handleCollisions(cars, innerBorder.get(), outerBorder.get());

        render(window, cars, carShapes, coordTransform, innerBorder.get(), outerBorder.get());
    }

    return 0;
}
