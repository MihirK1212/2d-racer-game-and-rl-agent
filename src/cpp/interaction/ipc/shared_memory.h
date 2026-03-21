#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#pragma pack(push, 1)
struct SharedGameData {
    // Python -> C++ (input flags)
    uint8_t input_up;
    uint8_t input_down;
    uint8_t input_left;
    uint8_t input_right;

    // Python -> C++ (action ready + reset flag + done flag)
    uint8_t action_ready; // Python sets to 1 after writing action
    uint8_t reset_flag; // Python sets to 1 to request episode reset
    
    // C++ -> Python (state flag + car state)
    uint8_t state_ready; // C++ sets to 1 after writing state
    uint8_t done_flag; // C++ sets to 1 when episode ends
    
    // uint8_t lap_count;
    // uint8_t checkpoints_crossed;
    // uint8_t rank_in_race;
    // uint8_t collided_in_this_frame;
    // uint8_t _pad2[4];
    // double angle_on_track;
    // double total_progress;
    double pos_x;
    double pos_y;
    double speed;
    double dir_x;
    double dir_y; 

    // uint8_t opponent_lap_count;
    // uint8_t opponent_checkpoints_crossed;
    // uint8_t opponent_rank_in_race;
    // uint8_t opponent_collided_in_this_frame;
    // uint8_t _pad3[4];
    // double opponent_angle_on_track;
    // double opponent_total_progress;
    // double opponent_pos_x;
    // double opponent_pos_y;
    // double opponent_speed;
    // double opponent_dir_x;
    // double opponent_dir_y; 

    // double dist_to_inner_wall;
    // double dist_to_outer_wall; 
    // double heading_vs_tangent; // angle between heading and tangent of the track
};
#pragma pack(pop)

static_assert(sizeof(SharedGameData) == 59, "SharedGameData must be exactly 56 bytes");

constexpr const char* SHM_NAME = "/RacerGameSHM";
constexpr size_t SHM_SIZE = sizeof(SharedGameData);

class SharedGameMemory {
    int shm_fd = -1;
    SharedGameData* data = nullptr;

public:

    SharedGameMemory()
    {
        shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1)
            throw std::runtime_error("shm_open failed");

        if (ftruncate(shm_fd, SHM_SIZE) == -1) {
            close(shm_fd);
            throw std::runtime_error("ftruncate failed");
        }

        data = static_cast<SharedGameData*>(
            mmap(nullptr, SHM_SIZE,
                 PROT_READ | PROT_WRITE,
                 MAP_SHARED, shm_fd, 0));

        if (data == MAP_FAILED) {
            close(shm_fd);
            throw std::runtime_error("mmap failed");
        }

        std::memset(data, 0, SHM_SIZE);
        std::cout << "Shared memory created: " << SHM_NAME << "\n";
    }

    ~SharedGameMemory()
    {
        if (data && data != MAP_FAILED)
            munmap(data, SHM_SIZE);

        if (shm_fd != -1)
            close(shm_fd);

        shm_unlink(SHM_NAME);
    }

    // purposley delete the copy and move constructors and operators
    // this prevents double free errors
    SharedGameMemory(const SharedGameMemory&) = delete;
    SharedGameMemory& operator=(const SharedGameMemory&) = delete;
    SharedGameMemory(SharedGameMemory&&) = delete;
    SharedGameMemory& operator=(SharedGameMemory&&) = delete;

    SharedGameData* getData()
    {
        return data;
    }
};

#endif
