cmake_minimum_required(VERSION 3.21)

project(racer_engine LANGUAGES CXX)

# ---- C++ Standard ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Build type default ----
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---- Compiler warnings (good for engine code) ----
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
)

# ---- Find SFML 3 ----
# Looks in /usr/local where you installed it
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

# ---- Source files ----
file(GLOB_RECURSE SRC_FILES
    src/cpp/*.cpp
)

# ---- Executable ----
add_executable(racer ${SRC_FILES})

# ---- Include dirs ----
target_include_directories(racer PRIVATE
    src/cpp
)

# ---- Link SFML ----
target_link_libraries(racer PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)

# ---- Threads (future multithreading / RL workers) ----
find_package(Threads REQUIRED)
target_link_libraries(racer PRIVATE Threads::Threads)

# ---- Optimization presets ----
target_compile_options(racer PRIVATE
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Debug>:-O0 -g>
)


