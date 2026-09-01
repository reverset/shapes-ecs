cmake_minimum_required(VERSION 3.22)
project(game)

if(EMSCRIPTEN)
    set(PLATFORM Web CACHE STRING "" FORCE)

    set(CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES "")
    set(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES "")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2")

include(FetchContent)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)

add_executable(game
        src/game/main.cpp
        src/game/particles.h
        src/game/BitLayers.h
        src/game/enemies.h
        src/game/unitcomponents.h
        src/game/assetstore.h
        src/engine/Universe.cpp
        src/engine/anim.h
        src/engine/event.h
        src/engine/Universe.h
        src/engine/vec.h
        src/engine/timer.h
        src/engine/Files.h
        src/engine/resource.h
        src/engine/util.h
        src/engine/inputsys.h
        src/engine/RandomGen.cpp
        src/engine/RandomGen.h
        src/engine/ecs.h
        src/engine/logging.h
        src/engine/standardcomponents.h
        src/engine/tilemap.h
        src/engine/ui.h
        src/engine/renderutil.h
        src/engine/ecs2.h
)

target_link_libraries(game PRIVATE raylib)

if(EMSCRIPTEN)
    set_target_properties(game PROPERTIES SUFFIX ".html")

    # Add linker flags: embed resources and Emscripten options
    set_target_properties(game PROPERTIES
            LINK_FLAGS "--embed-file ${CMAKE_SOURCE_DIR}/resources@/resources -sUSE_GLFW=3 -sASYNCIFY -sALLOW_MEMORY_GROWTH"
    )

#    set(CMAKE_EXECUTABLE_SUFFIX ".html")
#    target_link_options(game PRIVATE
#            "-sUSE_GLFW=3"
#            "-sASYNCIFY"
#            "-sALLOW_MEMORY_GROWTH"
#            "--embed-file ${CMAKE_SOURCE_DIR}/resources@/resources"
#    )
endif()
