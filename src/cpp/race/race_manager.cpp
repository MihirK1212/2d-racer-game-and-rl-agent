#include "race_manager.h"

RaceManager::RaceManager(TrackProgress& trackProgress,
                         const std::vector<Car*>& cars,
                         const std::vector<CarStartConfig>& startConfigs)
    : trackProgress(trackProgress), cars(cars), startConfigs(startConfigs),
      wrongWayFrameCounts(cars.size(), 0)
{
    resetRace();
}

void RaceManager::resetAndStartRace() {
    resetRace();
    state = RaceState::RACING;
}

void RaceManager::onSpacePressed() {
    if (state == RaceState::WAITING || state == RaceState::FINISHED) {
        resetAndStartRace();
    }
}

void RaceManager::updateRaceProgress() {
    if (state != RaceState::RACING) {
        return;
    }

    for (size_t i = 0; i < cars.size(); ++i) {
        trackProgress.update(static_cast<int>(i), cars[i]->getPosition());
    }
}

void RaceManager::updateRaceOutcome() {
    if (state != RaceState::RACING) {
        return;
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

void RaceManager::handleWrongWay() {
    if (state != RaceState::RACING) return;

    for (size_t i = 0; i < cars.size(); ++i) {
        const CarProgress& cp = trackProgress.getCarProgress(static_cast<int>(i));
        if (cp.goingWrongWay) {
            wrongWayFrameCounts[i]++;
            if (wrongWayFrameCounts[i] > WRONG_WAY_RESPAWN_FRAMES) {
                respawnCar(static_cast<int>(i));
                wrongWayFrameCounts[i] = 0;
            }
        } else {
            wrongWayFrameCounts[i] = 0;
        }
    }
}

void RaceManager::updateRace() {
    updateRaceProgress();
    handleWrongWay();
    updateRaceOutcome();
}

void RaceManager::resetRace() {
    state = RaceState::WAITING;
    winnerCarIndex = -1;
    std::fill(wrongWayFrameCounts.begin(), wrongWayFrameCounts.end(), 0);

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

bool RaceManager::isRacing() const {
    return state == RaceState::RACING;
}

bool RaceManager::isIdleState() const {
    return state == RaceState::WAITING || state == RaceState::FINISHED;
}