#include "Universe.h"

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>

#include "GameObject.h"
#include "raylib.h"

namespace Universe {
    std::vector<GameObject*> game_objects; // unique pointers?
    std::vector<std::function<void()>> deferred_actions;
    Camera2D camera;

    bool shutdown = false;
    double gameTime = 0.0;
    double timeScale = 1.0;
    ResourceManager* resourceManager;

    std::vector<GameObject*>& getGameObjects() {
        return game_objects;
    }

    void defer(const std::function<void()> &f) {
        deferred_actions.push_back(f);
    }

    void runDeferred() {
        while (!deferred_actions.empty()) {
            auto last = deferred_actions.back();
            deferred_actions.pop_back();
            last();
        }
    }

    void renderAll() {
        BeginDrawing();
        ClearBackground(WHITE);
        BeginMode2D(camera);

        for (const auto game_object : game_objects) {
            game_object->render2d();
        }

        EndMode2D();

        DrawFPS(15, 15);
        EndDrawing();
    }

    void updateAll() {
        for (const auto game_object : game_objects) {
            game_object->update();
        }
    }

    void deInit(const std::function<void()>& stop) {
        std::cout << "Shutting down ..." << std::endl;
        for (const auto object : game_objects) {
            delete object;
        }
        delete resourceManager;

        stop();

        CloseWindow();
    }

    void runBlocking() {
        while (!WindowShouldClose() && !shutdown) {
            updateAll();
            renderAll();
            runDeferred();

            gameTime += getScaledDeltaTime();
        }
    }

    void init(const int width, const int height, const char* title, const std::function<void()>& start, const std::function<void()>& stop) {
        camera = {
            .offset = {static_cast<float>(width)/2.0f, static_cast<float>(height)/2.0f},
            .target = {0, 0},
            .rotation = 0.0f,
            .zoom = 1.0f,
        };

        resourceManager = new ResourceManager;

        InitWindow(width, height, title);

        start();

        runBlocking();
        deInit(stop);
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

    ResourceManager* getResourceManager() {
         return resourceManager;
    }
} // Universe