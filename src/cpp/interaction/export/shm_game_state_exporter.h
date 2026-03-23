#ifndef SHM_GAME_STATE_EXPORTER_H
#define SHM_GAME_STATE_EXPORTER_H

#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>

#include "../../entity/car/car.h"
#include "../ipc/shared_memory.h"
#include "../../race/track_progress.h"
#include "../../race/race_manager.h"

struct CollisionStateResult
{
    int collidedWithCar = -1;
    bool collidedWithInnerBorder = false;
    bool collidedWithOuterBorder = false;
};

class SHMGameStateExporter
{
    SharedGameMemory &sgm;

    static constexpr double PI = 3.14159265358979323846;

    static constexpr double RAY_ANGLES_DEG[NUM_RAYCASTS] = {
        -90.0, -60.0, -30.0, 0.0, 30.0, 60.0, 90.0
    };

    static double castRayAgainstPolyline(
        const Vector2D &origin,
        const Vector2D &dir,
        const std::vector<Curve2DPoint> &pts
    ) {
        double best = MAX_RAY_DISTANCE;
        for (size_t i = 0; i + 1 < pts.size(); ++i) {
            const Vector2D &a = pts[i].position;
            const Vector2D &b = pts[i + 1].position;
            Vector2D r = b - a;
            Vector2D q = origin - a;
            double denom = r.x * dir.y - r.y * dir.x;
            if (std::abs(denom) < 1e-9) continue;
            double t = (r.x * q.y - r.y * q.x) / denom;
            double s = (dir.y * q.x - dir.x * q.y) / denom;
            if (t >= 0.0 && s >= 0.0 && s <= 1.0 && t < best)
                best = t;
        }
        return best;
    }

    void writeSelfCarState(
        SharedGameData *data,
        const Car &car,
        const CollisionStateResult &collision,
        const CarProgress &progress,
        int rank,
        const ParametricCurve2D *innerBorder,
        const ParametricCurve2D *outerBorder,
        const ParametricCurve2D *centerline
    ) {
        Vector2D pos = car.getPosition();
        Vector2D dir = car.getDirection();

        data->lap_count = static_cast<uint8_t>(progress.lapsCompleted);
        data->checkpoints_crossed = static_cast<uint8_t>(progress.nextCheckpoint);
        data->rank_in_race = static_cast<uint8_t>(rank);
        data->collided_in_this_frame = static_cast<uint8_t>(
            collision.collidedWithCar >= 0 ||
            collision.collidedWithInnerBorder ||
            collision.collidedWithOuterBorder
        );
        data->angle_on_track = progress.currentTheta;
        data->total_progress = progress.totalProgress;
        data->pos_x = pos.x;
        data->pos_y = pos.y;
        data->speed = car.getSpeed();
        data->dir_x = dir.x;
        data->dir_y = dir.y;

        data->dist_to_inner_wall = innerBorder->closestPointTo(pos).distance;
        data->dist_to_outer_wall = outerBorder->closestPointTo(pos).distance;

        /*
        computes the signed angle between the car's heading direction and the centerline tangent at the car's current theta, using atan2(cross, dot).
        A value of 0 means the car is perfectly aligned with the track; positive/negative values indicate deviation direction.
        */
        Vector2D tangent = centerline->tangentAt(progress.currentTheta);
        double dot = Vector2D::dot(dir, tangent);
        double cross = Vector2D::cross(dir, tangent);
        data->heading_vs_tangent = std::atan2(cross, dot);

        const auto &innerPts = innerBorder->getCachedPoints();
        const auto &outerPts = outerBorder->getCachedPoints();
        for (int i = 0; i < NUM_RAYCASTS; ++i) {
            double angleRad = RAY_ANGLES_DEG[i] * (PI / 180.0);
            Vector2D rayDir = dir.rotate(angleRad);
            double dInner = castRayAgainstPolyline(pos, rayDir, innerPts);
            double dOuter = castRayAgainstPolyline(pos, rayDir, outerPts);
            data->ray_distances[i] = std::min(dInner, dOuter);
        }
    }

    void writeOpponentCarState(
        SharedGameData *data,
        const Car &car,
        const CollisionStateResult &collision,
        const CarProgress &progress,
        int rank
    ) {
        Vector2D pos = car.getPosition();
        Vector2D dir = car.getDirection();

        data->opponent_lap_count = static_cast<uint8_t>(progress.lapsCompleted);
        data->opponent_checkpoints_crossed = static_cast<uint8_t>(progress.nextCheckpoint);
        data->opponent_rank_in_race = static_cast<uint8_t>(rank);
        data->opponent_collided_in_this_frame = static_cast<uint8_t>(
            collision.collidedWithCar >= 0 ||
            collision.collidedWithInnerBorder ||
            collision.collidedWithOuterBorder
        );
        data->opponent_angle_on_track = progress.currentTheta;
        data->opponent_total_progress = progress.totalProgress;
        data->opponent_pos_x = pos.x;
        data->opponent_pos_y = pos.y;
        data->opponent_speed = car.getSpeed();
        data->opponent_dir_x = dir.x;
        data->opponent_dir_y = dir.y;
    }

    public:
    explicit SHMGameStateExporter(SharedGameMemory &sgm) : sgm(sgm) {}

    void exportState(
        int selfCarIndex,
        const std::vector<std::unique_ptr<Car>> &cars,
        const std::vector<CollisionStateResult> &collisionStateResults,
        const TrackProgress &trackProgress, const RaceManager &raceManager,
        const ParametricCurve2D *innerBorder, const ParametricCurve2D *outerBorder, const ParametricCurve2D *centerline
    )
    {
        SharedGameData *data = sgm.getData();
        if (!data) return;

        int oppCarIndex = 1 - selfCarIndex;

        int selfRank = (raceManager.getLeaderIndex() == selfCarIndex) ? 1 : 2;
        int oppRank  = (raceManager.getLeaderIndex() == oppCarIndex)  ? 1 : 2;

        writeSelfCarState(data, *cars[selfCarIndex],
            collisionStateResults[selfCarIndex],
            trackProgress.getCarProgress(selfCarIndex),
            selfRank, innerBorder, outerBorder, centerline);

        writeOpponentCarState(data, *cars[oppCarIndex],
            collisionStateResults[oppCarIndex],
            trackProgress.getCarProgress(oppCarIndex),
            oppRank);

        data->done_flag = static_cast<uint8_t>(raceManager.getState() == RaceState::FINISHED);
        data->state_ready = 1;
    }
};

#endif
