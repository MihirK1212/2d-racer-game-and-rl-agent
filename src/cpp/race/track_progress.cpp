#include "track_progress.h"
#include <cmath>
#include <algorithm>

TrackProgress::TrackProgress(float centerlineRadius, float straightLength, int numCars)
    : centerline(std::make_unique<RoundedRectangleCurve>(centerlineRadius, straightLength)),
      carsProgress(numCars)
{
    centerline->generate(0, 360, 720);

    checkpointThetas.reserve(NUM_CHECKPOINTS);
    for (int i = 0; i < NUM_CHECKPOINTS; ++i) {
        checkpointThetas.push_back(i * CHECKPOINT_INTERVAL);
    }
}

double TrackProgress::normalizeTheta(double theta) const {
    theta = std::fmod(theta, 360.0);
    if (theta < 0.0) theta += 360.0;
    return theta;
}

double TrackProgress::computeDelta(double current, double previous) const {
    double delta = current - previous;
    if (delta > 180.0) delta -= 360.0;
    if (delta < -180.0) delta += 360.0;
    return delta;
}

void TrackProgress::updateCheckpoints(int carIndex, double delta) {
    if (delta <= 0.0) return;

    CarProgress& cp = carsProgress[carIndex];
    double prev = cp.previousTheta;
    double curr = cp.currentTheta;

    double nextCpTheta = checkpointThetas[cp.nextCheckpoint];

    bool crossed = false;
    if (prev <= curr) {
        crossed = (prev < nextCpTheta && nextCpTheta <= curr);
    } else {
        crossed = (prev < nextCpTheta || nextCpTheta <= curr);
    }

    if (crossed) {
        cp.nextCheckpoint = (cp.nextCheckpoint + 1) % NUM_CHECKPOINTS;
    }
}

void TrackProgress::initializeCar(int carIndex, const Vector2D& position) {
    ClosestPointResult result = centerline->closestPointTo(position);
    double theta = normalizeTheta(result.theta);

    CarProgress& cp = carsProgress[carIndex];
    cp.currentTheta = theta;
    cp.previousTheta = theta;
    cp.lapsCompleted = 0;
    cp.nextCheckpoint = 0;
    cp.goingWrongWay = false;
    cp.totalProgress = 0.0;

    std::cout << "for car " << carIndex << ", initialization theta: " << theta << std::endl;

    // Advance nextCheckpoint past the starting theta so checkpoint 0 (start line)
    // is the LAST checkpoint to cross for lap completion
    for (int i = 0; i < NUM_CHECKPOINTS; ++i) {
        if (checkpointThetas[i] <= theta) {
            cp.nextCheckpoint = (i + 1) % NUM_CHECKPOINTS;
        }
    }

    initialized = true;
}

void TrackProgress::update(int carIndex, const Vector2D& position) {
    ClosestPointResult closestPointResult = centerline->closestPointTo(position);
    CarProgress& cp = carsProgress[carIndex];

    cp.previousTheta = cp.currentTheta;
    cp.currentTheta = normalizeTheta(closestPointResult.theta);

    double delta = computeDelta(cp.currentTheta, cp.previousTheta);

    cp.goingWrongWay = (delta < WRONG_WAY_THRESHOLD);

    // Lap completion must be checked BEFORE checkpoint update because
    // checkpoint 0 sits at the start line and would advance nextCheckpoint
    // away from 0 in the same frame, preventing the lap from being counted.
    if (delta > 0.0 && cp.nextCheckpoint == 0) {
        bool crossedStart = (cp.previousTheta > 300.0 && cp.currentTheta < 60.0);
        if (crossedStart) {
            cp.lapsCompleted++;
        }
    }

    updateCheckpoints(carIndex, delta);

    cp.totalProgress = cp.lapsCompleted * 360.0 + cp.currentTheta;
}

void TrackProgress::reset(int carIndex, const Vector2D& position) {
    initializeCar(carIndex, position);
}

const CarProgress& TrackProgress::getCarProgress(int carIndex) const {
    return carsProgress[carIndex];
}

int TrackProgress::getRanking(int carIndexA, int carIndexB) const {
    if (carsProgress[carIndexA].totalProgress >= carsProgress[carIndexB].totalProgress)
        return 0;
    return 1;
}
