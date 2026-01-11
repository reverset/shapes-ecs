#include "Universe.h"

#include <string>
#include <vector>
#include <functional>
#include <algorithm>

#include "GameObject.h"
#include "raylib.h"

namespace Universe {
    std::vector<GameObject*>* game_objects; // unique pointers?
    Camera2D camera;

    bool shutdown = false;
    double gameTime = 0.0;
    double timeScale = 1.0;

    std::vector<GameObject*>& getGameObjects() {
        return *game_objects;
    }

    void renderAll() {
        BeginDrawing();
        ClearBackground(WHITE);
        BeginMode2D(camera);

        for (const auto game_object : *game_objects) {
            game_object->render2d();
        }

        EndMode2D();

        DrawFPS(15, 15);
        EndDrawing();
    }

    void updateAll() {
        for (const auto game_object : *game_objects) {
            game_object->update();
        }
    }

    void deInit() {
        for (const GameObject* object : *game_objects) {
            delete object;
        }

        CloseWindow();
        delete game_objects;
    }

    void runBlocking() {
        while (!WindowShouldClose() && !shutdown) {
            updateAll();
            renderAll();

            gameTime += getScaledDeltaTime();
        }
    }

    void init(const int width, const int height, const char* title, const std::function<void()>& start) {
        game_objects = new std::vector<GameObject*>();
        camera = {
            .offset = {static_cast<float>(width)/2.0f, static_cast<float>(height)/2.0f},
            .target = {0, 0},
            .rotation = 0.0f,
            .zoom = 1.0f,
        };

        InitWindow(width, height, title);

        start();

        runBlocking();
        deInit();
    }

    Camera2D* getCamera() {
        return &camera;
    }

    double getTimeScale() {
        return timeScale;
    }

    float getScaledDeltaTime() {
        return GetFrameTime() * static_cast<float>(timeScale);
    }

    double getGameTime() {
        return gameTime;
    }
} // Universe