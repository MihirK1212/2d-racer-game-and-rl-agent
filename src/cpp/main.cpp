#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <ctime>
#include <cmath>
#include <optional>
#include <string>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#include "./entity/car/car.h"
#include "./engine/vector.h"
#include "./engine/coordinates.h"
#include "./engine/collision/collision.h"
#include "./interaction/input/keyboard_car_input_handler.h"
#include "./interaction/input/shm_car_input_handler.h"
#include "./interaction/input/centerline_car_input_handler.h"
#include "./interaction/export/console_car_state_exporter.h"
#include "./interaction/export/shm_game_state_exporter.h"
#include "./interaction/ipc/shared_memory.h"
#include "./interaction/synchronizer.h"
#include "./entity/curve/rounded_rectangle_curve.h"
#include "./race/track_progress.h"
#include "./race/race_manager.h"
#include "./race/hud.h"

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

sf::RectangleShape createCarShape(const Car &car, CoordinateTransform &coordTransform, sf::Color color)
{
    sf::RectangleShape shape({static_cast<float>(coordTransform.metersToPixels(car.getWidth())),
                              static_cast<float>(coordTransform.metersToPixels(car.getHeight()))});

    shape.setFillColor(color);

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
               ParametricCurve2D *curve,
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
                      ParametricCurve2D *innerBorder,
                      ParametricCurve2D *outerBorder,
                      std::vector<CollisionStateResult> &collisionStateResults)
{
    size_t numCars = cars.size();

    for (size_t i = 0; i < numCars; i++)
    {
        Car *car = cars[i].get();

        CollisionResult resultInner = detectCarVsCurve(car, innerBorder);
        if (resultInner.collided) {
            resolveCarVsCurve(car, innerBorder, resultInner);
            collisionStateResults[i].collidedWithInnerBorder = true;
        }

        CollisionResult resultOuter = detectCarVsCurve(car, outerBorder);
        if (resultOuter.collided) {
            resolveCarVsCurve(car, outerBorder, resultOuter);
            collisionStateResults[i].collidedWithOuterBorder = true;
        }
    }

    for (size_t i = 0; i < numCars; i++)
    {
        Car *car1 = cars[i].get();

        for (size_t j = i + 1; j < numCars; j++)
        {
            Car *car2 = cars[j].get();

            CollisionResult result = detectCarVsCar(car1, car2);
            if (result.collided) {
                resolveCarVsCar(car1, car2, result);
                collisionStateResults[i].collidedWithCar = static_cast<int>(j); 
                collisionStateResults[j].collidedWithCar = static_cast<int>(i);
            }
        }
    }
}

void drawStartFinishLine(sf::RenderWindow &window,
                         ParametricCurve2D *innerBorder,
                         ParametricCurve2D *outerBorder,
                         CoordinateTransform &coordTransform)
{
    Vector2D innerPt = innerBorder->getCachedPoints().front().position;
    Vector2D outerPt = outerBorder->getCachedPoints().front().position;

    Vector2D screenInner = coordTransform.gameToScreenPoint(innerPt);
    Vector2D screenOuter = coordTransform.gameToScreenPoint(outerPt);

    sf::VertexArray line(sf::PrimitiveType::Lines, 2);
    line[0] = sf::Vertex{{static_cast<float>(screenInner.x), static_cast<float>(screenInner.y)},
                          sf::Color(255, 255, 0)};
    line[1] = sf::Vertex{{static_cast<float>(screenOuter.x), static_cast<float>(screenOuter.y)},
                          sf::Color(255, 255, 0)};
    window.draw(line);
}

void handleWrongWayDetection(RaceManager& raceManager, TrackProgress& trackProgress, const std::vector<std::unique_ptr<Car>>& cars, std::vector<int>& wrongWayFrameCounts, int WRONG_WAY_RESPAWN_FRAMES) {
    if (raceManager.shouldAcceptInput()) {
        for (size_t i = 0; i < cars.size(); ++i) {
            const CarProgress& cp = trackProgress.getCarProgress(static_cast<int>(i));

            if (cp.goingWrongWay) {
                wrongWayFrameCounts[i]++;
                if (wrongWayFrameCounts[i] > WRONG_WAY_RESPAWN_FRAMES) {
                    raceManager.respawnCar(static_cast<int>(i));
                    wrongWayFrameCounts[i] = 0;
                }
            } else {
                wrongWayFrameCounts[i] = 0;
            }
        }
    } else {
        std::fill(wrongWayFrameCounts.begin(), wrongWayFrameCounts.end(), 0);
    }
}

void render(sf::RenderWindow &window,
            const std::vector<std::unique_ptr<Car>> &cars,
            std::vector<sf::RectangleShape> &carShapes,
            CoordinateTransform &coordTransform,
            ParametricCurve2D *innerBorder,
            ParametricCurve2D *outerBorder)
{
    window.clear(sf::Color::Black);

    drawStartFinishLine(window, innerBorder, outerBorder, coordTransform);

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
}

int main()
{
    constexpr unsigned int SCREEN_WIDTH = 1000;
    constexpr unsigned int SCREEN_HEIGHT = 600;
    constexpr int WRONG_WAY_RESPAWN_FRAMES = 10;

    constexpr float INNER_RADIUS = 20.0f;
    constexpr float OUTER_RADIUS = 28.0f;
    constexpr float STRAIGHT_LENGTH = 40.0f;
    constexpr float CENTERLINE_RADIUS = (INNER_RADIUS + OUTER_RADIUS) / 2.0f;

    bool externalInputMode = true;
    bool stepMode = true;

    sf::RenderWindow window = createWindow();

    // Cars start on the track at theta=0 (rightmost point), facing up (+y = counterclockwise)
    Vector2D startPos0(STRAIGHT_LENGTH / 2.0 + INNER_RADIUS + 2.0, -0.5);
    Vector2D startPos1(STRAIGHT_LENGTH / 2.0 + OUTER_RADIUS - 2.0, -0.5);
    Vector2D startDir(0, 1);

    std::vector<std::unique_ptr<Car>> cars;
    cars.push_back(std::make_unique<Car>(startPos0.x, startPos0.y, 0.7, 1.5));
    cars.push_back(std::make_unique<Car>(startPos1.x, startPos1.y, 0.7, 1.5));

    CoordinateTransform coordTransform(SCREEN_WIDTH, SCREEN_HEIGHT);

    auto innerBorder = std::make_unique<RoundedRectangleCurve>(INNER_RADIUS, STRAIGHT_LENGTH);
    auto outerBorder = std::make_unique<RoundedRectangleCurve>(OUTER_RADIUS, STRAIGHT_LENGTH);
    auto centerline  = std::make_unique<RoundedRectangleCurve>(CENTERLINE_RADIUS, STRAIGHT_LENGTH);

    innerBorder->generate(0, 360, 200);
    outerBorder->generate(0, 360, 200);
    centerline->generate(0, 360, 200);

    SharedGameMemory shm;

    auto synchronizer = std::make_unique<CarSynchronizer>(externalInputMode, stepMode, shm);

    std::unique_ptr<CarInputHandler> inputHandler1;
    if (externalInputMode && stepMode) {
        inputHandler1 = std::make_unique<CenterlineCarInputHandler>(centerline.get());
    } else {
        inputHandler1 = std::make_unique<KeyboardCarInputHandler>();
    }

    std::unique_ptr<CarInputHandler> inputHandler2;
    if(externalInputMode) {
        inputHandler2 = std::make_unique<SHMCarInputHandler>(shm);
    } else {
        CarKeyBindings keyBindings = {
            sf::Keyboard::Key::A,
            sf::Keyboard::Key::D,
            sf::Keyboard::Key::W,
            sf::Keyboard::Key::S,
        };
        inputHandler2 = std::make_unique<KeyboardCarInputHandler>(keyBindings);
    }

    std::unique_ptr<SHMGameStateExporter> shmGameStateExporter = std::make_unique<SHMGameStateExporter>(shm);

    // Race systems
    TrackProgress trackProgress(centerline.get(), 2);
    trackProgress.initializeCar(0, startPos0);
    trackProgress.initializeCar(1, startPos1);

    std::vector<CarStartConfig> startConfigs = {
        {startPos0, startDir},
        {startPos1, startDir}
    };
    std::vector<Car*> carPtrs = {cars[0].get(), cars[1].get()};

    RaceManager raceManager(trackProgress, carPtrs, startConfigs);

    HUD hud("arial.ttf", SCREEN_WIDTH, SCREEN_HEIGHT);

    std::vector<sf::RectangleShape> carShapes;
    carShapes.push_back(createCarShape(*cars[0], coordTransform, sf::Color::Red));
    carShapes.push_back(createCarShape(*cars[1], coordTransform, sf::Color(80, 140, 255)));
    std::vector<int> wrongWayFrameCounts(cars.size(), 0);

    std::vector<CollisionStateResult> collisionStateResults;
    collisionStateResults.resize(cars.size());
    for(size_t i = 0; i < cars.size(); i++) {
        collisionStateResults[i].collidedWithCar = -1;
        collisionStateResults[i].collidedWithInnerBorder = false;
        collisionStateResults[i].collidedWithOuterBorder = false;
    }

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* keyEvt = event->getIf<sf::Event::KeyPressed>()) {
                if (keyEvt->code == sf::Keyboard::Key::Space)
                    raceManager.onSpacePressed();
                if (keyEvt->code == sf::Keyboard::Key::R) {
                    raceManager.respawnCar(0);
                    wrongWayFrameCounts[0] = 0;
                }
            }
        }

        shmGameStateExporter->exportState(*cars[0], *cars[1], collisionStateResults, trackProgress, raceManager, innerBorder.get(), outerBorder.get(), centerline.get());

        if(synchronizer->isStepMode()) {
            while(!synchronizer->isActionReady()){
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        if (raceManager.shouldAcceptInput()) {
            inputHandler1->apply(*cars[0]);

            if(synchronizer->isExternalInputMode() && synchronizer->isActionReady()) {
                inputHandler2->apply(*cars[1]);
                synchronizer->setActionReady(false);
            }
            else if(!synchronizer->isExternalInputMode()) {
                inputHandler2->apply(*cars[1]);
            }
        }

        if(synchronizer->isStepMode() && synchronizer->isResetFlagSet()){
            raceManager.resetRace();
            synchronizer->setResetFlag(false);
        }

        if (raceManager.shouldAcceptInput()) {
            updateGame(*cars[0]);
            updateGame(*cars[1]);

            handleCollisions(cars, innerBorder.get(), outerBorder.get(), collisionStateResults);
        }

        raceManager.update();

        handleWrongWayDetection(raceManager, trackProgress, cars, wrongWayFrameCounts, WRONG_WAY_RESPAWN_FRAMES);

        render(window, cars, carShapes, coordTransform, innerBorder.get(), outerBorder.get());
        hud.draw(window, raceManager, trackProgress, 2);
        window.display();
    }

    return 0;
}
