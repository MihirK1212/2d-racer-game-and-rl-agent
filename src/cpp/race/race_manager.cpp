#include "race_manager.h"

RaceManager::RaceManager(TrackProgress& trackProgress,
                         const std::vector<Car*>& cars,
                         const std::vector<CarStartConfig>& startConfigs)
    : trackProgress(trackProgress), cars(cars), startConfigs(startConfigs)
{
    resetRace();
}

void RaceManager::onSpacePressed() {
    if (state == RaceState::WAITING) {
        state = RaceState::COUNTDOWN;
        countdownStep = 0;
        countdownClock.restart();
    } else if (state == RaceState::FINISHED) {
        resetRace();
    }
}

void RaceManager::update() {
    if (state == RaceState::COUNTDOWN) {
        float elapsed = countdownClock.getElapsedTime().asSeconds();
        countdownStep = static_cast<int>(elapsed / COUNTDOWN_STEP_SECONDS);

        if (countdownStep >= COUNTDOWN_STEPS) {
            state = RaceState::RACING;
        }
    }

    if (state == RaceState::RACING) {
        for (size_t i = 0; i < cars.size(); ++i) {
            trackProgress.update(static_cast<int>(i), cars[i]->getPosition());
        }

        int bestFinisher = -1;
        double bestProgress = -1.0;
        for (size_t i = 0; i < cars.size(); ++i) {
            const CarProgress& cp = trackProgress.getCarProgress(static_cast<int>(i));
            if (cp.lapsCompleted >= TOTAL_LAPS && cp.totalProgress > bestProgress) {
                bestProgress = cp.totalProgress;
                bestFinisher = static_cast<int>(i);
            }
        }
        if (bestFinisher >= 0) {
            state = RaceState::FINISHED;
            winnerCarIndex = bestFinisher;
        }
    }
}

void RaceManager::resetRace() {
    state = RaceState::WAITING;
    countdownStep = 0;
    winnerCarIndex = -1;

    for (size_t i = 0; i < cars.size(); ++i) {
        cars[i]->reset(startConfigs[i].position, startConfigs[i].direction);
        trackProgress.reset(static_cast<int>(i), startConfigs[i].position);
    }
}

void RaceManager::respawnCar(int carIndex) {
    if (state != RaceState::RACING) return;

    RespawnInfo info = trackProgress.getRespawnInfo(carIndex);
    cars[carIndex]->reset(info.position, info.direction);
    trackProgress.respawnCar(carIndex);
}

RaceState RaceManager::getState() const {
    return state;
}

int RaceManager::getCountdownNumber() const {
    // countdownStep: 0->"3", 1->"2", 2->"1", 3->"GO!"
    if (countdownStep < 3)
        return 3 - countdownStep;
    return 0; // 0 means "GO!"
}

int RaceManager::getWinnerIndex() const {
    return winnerCarIndex;
}

int RaceManager::getTotalLaps() const {
    return TOTAL_LAPS;
}

int RaceManager::getLeaderIndex() const {
    int leader = 0;
    double best = trackProgress.getCarProgress(0).totalProgress;
    for (size_t i = 1; i < cars.size(); ++i) {
        double p = trackProgress.getCarProgress(static_cast<int>(i)).totalProgress;
        if (p > best) {
            best = p;
            leader = static_cast<int>(i);
        }
    }
    return leader;
}

bool RaceManager::shouldAcceptInput() const {
    return state == RaceState::RACING;
}
