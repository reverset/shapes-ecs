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
        main.cpp
        Universe.cpp
        Universe.h
        GameObject.h
        vec.h
        timer.h
        Files.h
        resource.h
        util.h
        UIObject.h
        inputsys.h
        ParticleSystem.h
        RandomGen.cpp
        RandomGen.h
        ecs.h
        logging.h
        iter.h
)

target_link_libraries(game PRIVATE raylib)
