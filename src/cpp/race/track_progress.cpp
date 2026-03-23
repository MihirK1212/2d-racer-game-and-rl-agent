#include "track_progress.h"
#include <cmath>
#include <algorithm>
#include <iostream>

TrackProgress::TrackProgress(ParametricCurve2D* centerline, int numCars)
    : centerline(centerline),
      carsProgress(numCars)
{
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

double TrackProgress::forwardThetaDistance(double fromTheta, double toTheta) const {
    return normalizeTheta(toTheta - fromTheta);
}

int TrackProgress::getCompletedCheckpointsThisLap(const CarProgress& cp) const {
    return (cp.nextCheckpoint + NUM_CHECKPOINTS - 1) % NUM_CHECKPOINTS;
}

double TrackProgress::computeValidatedProgress(const CarProgress& cp) const {
    const int completedCheckpoints = getCompletedCheckpointsThisLap(cp);
    const int lastCheckpoint = completedCheckpoints;

    const double baseProgress =
        cp.lapsCompleted * 360.0 + completedCheckpoints * CHECKPOINT_INTERVAL;

    const double lastCheckpointTheta = checkpointThetas[lastCheckpoint];
    const double nextCheckpointTheta = checkpointThetas[cp.nextCheckpoint];
    const double segmentSpan = forwardThetaDistance(lastCheckpointTheta, nextCheckpointTheta);
    const double segmentProgress = forwardThetaDistance(lastCheckpointTheta, cp.currentTheta);

    // Only count intra-segment progress while the car is still between the
    // last confirmed checkpoint and the next one. If it reverses behind the
    // last checkpoint, clamp to the checkpoint boundary instead of wrapping
    // around and incorrectly inflating progress.
    if (segmentProgress <= segmentSpan) {
        return baseProgress + segmentProgress;
    }

    return baseProgress;
}

void TrackProgress::updateLapsAndCheckpoints(int carIndex, double delta) {
    if (delta <= 0.0) return;

    CarProgress& cp = carsProgress[carIndex];
    double prev = cp.previousTheta;
    double curr = cp.currentTheta;

    for (int guard = 0; guard < NUM_CHECKPOINTS; ++guard) {
        double nextCpTheta = checkpointThetas[cp.nextCheckpoint];

        bool crossed = false;
        if (prev <= curr) {
            crossed = (prev < nextCpTheta && nextCpTheta <= curr);
        } else {
            crossed = (prev < nextCpTheta || nextCpTheta <= curr);
        }

        if (!crossed) break;

        if (cp.nextCheckpoint == 0 && cp.firstUpdateDone) {
            cp.lapsCompleted++;
        }

        cp.nextCheckpoint = (cp.nextCheckpoint + 1) % NUM_CHECKPOINTS;
    }
}

void TrackProgress::initializeCar(int carIndex, const Vector2D& position) {
    ClosestPointResult result = centerline->closestPointTo(position);
    double theta = normalizeTheta(result.theta);

    // Snap near-360 values to 0: a car at the start/finish line can get
    // theta just below 360 due to closestPointTo landing on the trailing
    // side of the 0/360 boundary.
    if (theta > 360.0 - START_LINE_SNAP_TOLERANCE) {
        theta = 0.0;
    }

    CarProgress& cp = carsProgress[carIndex];
    cp.currentTheta = theta;
    cp.previousTheta = theta;
    cp.lapsCompleted = 0;
    cp.nextCheckpoint = 0;
    cp.goingWrongWay = false;
    cp.firstUpdateDone = false;

    // Advance nextCheckpoint past the starting theta so checkpoint 0 (start line)
    // is the LAST checkpoint to cross for lap completion
    for (int i = 0; i < NUM_CHECKPOINTS; ++i) {
        if (checkpointThetas[i] <= theta) {
            cp.nextCheckpoint = (i + 1) % NUM_CHECKPOINTS;
        }
    }

    cp.totalProgress = computeValidatedProgress(cp);

    initialized = true;
}

void TrackProgress::update(int carIndex, const Vector2D& position) {
    ClosestPointResult closestPointResult = centerline->closestPointTo(position);
    CarProgress& cp = carsProgress[carIndex];

    cp.previousTheta = cp.currentTheta;
    cp.currentTheta = normalizeTheta(closestPointResult.theta);

    double delta = computeDelta(cp.currentTheta, cp.previousTheta);

    cp.goingWrongWay = (delta < WRONG_WAY_THRESHOLD);

    updateLapsAndCheckpoints(carIndex, delta);

    cp.firstUpdateDone = true;

    cp.totalProgress = computeValidatedProgress(cp);
}

void TrackProgress::reset(int carIndex, const Vector2D& position) {
    initializeCar(carIndex, position);
}

RespawnInfo TrackProgress::getRespawnInfo(int carIndex) const {
    const CarProgress& cp = carsProgress[carIndex];
    int lastCheckpoint = (cp.nextCheckpoint - 1 + NUM_CHECKPOINTS) % NUM_CHECKPOINTS;
    double theta = checkpointThetas[lastCheckpoint];

    Curve2DPoint point = centerline->getPointForTheta(theta);
    Vector2D direction = centerline->tangentAt(theta);

    return {point.position, direction};
}

void TrackProgress::respawnCar(int carIndex) {
    CarProgress& cp = carsProgress[carIndex];
    int lastCheckpoint = (cp.nextCheckpoint - 1 + NUM_CHECKPOINTS) % NUM_CHECKPOINTS;
    double theta = checkpointThetas[lastCheckpoint];

    cp.currentTheta = theta;
    cp.previousTheta = theta;
    cp.goingWrongWay = false;
    // A respawn happens mid-race, so the next start-line crossing should still
    // count toward lap completion when checkpoint 0 is pending.
    cp.firstUpdateDone = true;
    cp.totalProgress = computeValidatedProgress(cp);
}

const CarProgress& TrackProgress::getCarProgress(int carIndex) const {
    return carsProgress[carIndex];
}

int TrackProgress::getRanking(int carIndexA, int carIndexB) const {
    if (carsProgress[carIndexA].totalProgress >= carsProgress[carIndexB].totalProgress)
        return 0;
    return 1;
}
