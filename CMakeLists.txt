cmake_minimum_required(VERSION 3.22)
project(game)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)

add_executable(game
        src/main.cpp
        src/Universe.cpp
        src/Universe.h
        src/GameObject.h
        src/vec.h
        src/timer.h
        src/Files.h
        src/resource.h
        src/util.h
        src/UIObject.h
        src/inputsys.h
        src/ParticleSystem.h
        src/RandomGen.cpp
        src/RandomGen.h
        src/ecs.h
        src/logging.h
        src/iter.h
)

target_link_libraries(game PRIVATE raylib)
