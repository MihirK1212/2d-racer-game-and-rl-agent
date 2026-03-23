#ifndef SHM_GAME_STATE_EXPORTER_H
#define SHM_GAME_STATE_EXPORTER_H

#include <memory>
#include <vector>

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
