#ifndef RACE_MANAGER_H
#define RACE_MANAGER_H

#include <vector>
#include <memory>
#include <SFML/System.hpp>

#include "../entity/car/car.h"
#include "../engine/vector.h"
#include "track_progress.h"

enum class RaceState {
    WAITING,
    COUNTDOWN,
    RACING,
    FINISHED
};

struct CarStartConfig {
    Vector2D position;
    Vector2D direction;
};

class RaceManager {
    static constexpr int TOTAL_LAPS = 3;
    static constexpr float COUNTDOWN_STEP_SECONDS = 1.0f;
    static constexpr int COUNTDOWN_STEPS = 4; // "3", "2", "1", "GO!"

    RaceState state = RaceState::WAITING;
    sf::Clock countdownClock;
    int countdownStep = 0;
    int winnerCarIndex = -1;

    std::vector<CarStartConfig> startConfigs;
    TrackProgress& trackProgress;
    std::vector<Car*> cars;

public:
    RaceManager(TrackProgress& trackProgress,
                const std::vector<Car*>& cars,
                const std::vector<CarStartConfig>& startConfigs);

    void onSpacePressed();
    void update();

    void resetRace();

    RaceState getState() const;
    int getCountdownNumber() const;
    int getWinnerIndex() const;
    int getTotalLaps() const;
    int getLeaderIndex() const;

    bool shouldAcceptInput() const;
};

#endif
