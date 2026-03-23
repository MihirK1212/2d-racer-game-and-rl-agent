#ifndef RACE_MANAGER_H
#define RACE_MANAGER_H

#include <vector>
#include <memory>

#include "../entity/car/car.h"
#include "../engine/vector.h"
#include "track_progress.h"

enum class RaceState {
    WAITING,
    RACING,
    FINISHED
};

struct CarStartConfig {
    Vector2D position;
    Vector2D direction;
};

class RaceManager {
    static constexpr int TOTAL_LAPS = 3;
    static constexpr int WRONG_WAY_RESPAWN_FRAMES = 10;

    RaceState state = RaceState::WAITING;
    int winnerCarIndex = -1;

    TrackProgress& trackProgress;
    std::vector<Car*> cars;
    std::vector<CarStartConfig> startConfigs;
    std::vector<int> wrongWayFrameCounts;

    void updateRaceProgress();
    void handleWrongWay();
    void updateRaceOutcome();

public:
    RaceManager(TrackProgress& trackProgress,
                const std::vector<Car*>& cars,
                const std::vector<CarStartConfig>& startConfigs);

    void resetAndStartRace();
    void onSpacePressed();
    void updateRace();

    void resetRace();
    void respawnCar(int carIndex);

    RaceState getState() const;
    int getWinnerIndex() const;
    int getTotalLaps() const;
    int getLeaderIndex() const;

    bool isRacing() const;
    bool isIdleState() const;
};

#endif
