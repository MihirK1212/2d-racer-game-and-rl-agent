#ifndef TRACK_PROGRESS_H
#define TRACK_PROGRESS_H

#include <vector>
#include <memory>

#include "../engine/vector.h"
#include "../entity/curve/rounded_rectangle_curve.h"

struct CarProgress {
    double currentTheta = 0.0;
    double previousTheta = 0.0;
    int lapsCompleted = 0;
    int nextCheckpoint = 0;
    bool goingWrongWay = false;
    double totalProgress = 0.0;
    bool firstUpdateDone = false;
};

struct RespawnInfo {
    Vector2D position;
    Vector2D direction;
};

class TrackProgress {
    static constexpr int NUM_CHECKPOINTS = 8;
    static constexpr double CHECKPOINT_INTERVAL = 360.0 / NUM_CHECKPOINTS;
    static constexpr double WRONG_WAY_THRESHOLD = -0.5;
    static constexpr double START_LINE_SNAP_TOLERANCE = 1.0;

    std::unique_ptr<RoundedRectangleCurve> centerline;
    std::vector<double> checkpointThetas;
    std::vector<CarProgress> carsProgress;
    bool initialized = false;

    double normalizeTheta(double theta) const;
    double computeDelta(double current, double previous) const;
    double forwardThetaDistance(double fromTheta, double toTheta) const;
    int getCompletedCheckpointsThisLap(const CarProgress& cp) const;
    double computeValidatedProgress(const CarProgress& cp) const;
    void updateCheckpoints(int carIndex, double delta);

public:
    TrackProgress(float centerlineRadius, float straightLength, int numCars);

    void initializeCar(int carIndex, const Vector2D& position);
    void update(int carIndex, const Vector2D& position);
    void reset(int carIndex, const Vector2D& position);

    RespawnInfo getRespawnInfo(int carIndex) const;
    void respawnCar(int carIndex);

    const CarProgress& getCarProgress(int carIndex) const;
    int getRanking(int carIndexA, int carIndexB) const;
};

#endif
