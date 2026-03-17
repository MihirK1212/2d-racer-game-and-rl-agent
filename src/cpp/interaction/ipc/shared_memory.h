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

    // C++ -> Python (state flag + car state)
    uint8_t state_ready;
    uint8_t _pad[3];

    double pos_x;
    double pos_y;
    double speed;
    double dir_x;
    double dir_y;
    double tangential_accel;
};
#pragma pack(pop)

static_assert(sizeof(SharedGameData) == 56, "SharedGameData must be exactly 56 bytes");

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
