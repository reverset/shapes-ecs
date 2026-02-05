#include "Universe.h"

#include <iostream>
#include <vector>
#include <functional>

#include "raylib.h"
#include "inputsys.h"
#include "RandomGen.h"

#include "ecs.h"

namespace Universe {
    std::vector<std::function<void()>> deferredActions;

    EntityStorage entityStorage;

    EntityStorage& getEntityStorage() {
        return entityStorage;
    }

    Input input;

    bool isProcessing = false;

    int resolutionX, resolutionY;

    int getResolutionX() {
        return resolutionX;
    }

    int getResolutionY() {
        return resolutionY;
    }

    bool areEntitiesBusy() {
        return isProcessing;
    }

    Camera2D camera;

    bool shutdown = false;
    double gameTime = 0.0;
    double timeScale = 1.0;
    ResourceManager* resourceManager;

    Input *getInputManager() {
        return &input;
    }

    void defer(const std::function<void()> &f) {
        deferredActions.push_back(f);
    }

    void runDeferred() {
        onFinalFrameUpdate.tick();

        while (!deferredActions.empty()) {
            auto last = deferredActions.back();
            deferredActions.pop_back();
            last();
        }
    }

    void renderAll() {
        prepaint.tick();

        BeginDrawing();
        ClearBackground(DARKGRAY);
        BeginMode2D(camera);

        onEarlyRender2d.tick(); // more of a temporary solution.
        onRender2d.tick();
        onLateRender2d.tick();

        EndMode2D();

        onRenderUi.tick();

        DrawRectangle(0, 0, 150, 100, Fade(BLACK, 0.8));
        DrawFPS(15, 15);
        DrawText(GameUtil::fmt("entities: %d", getEntityStorage().size()).c_str(), 15, 35, 20, WHITE);

        EndDrawing();
    }

    void updateAll() {
        onUpdate.tick();
        onLateUpdate.tick();

        onIrregularUpdate.tick(getGameTime());
    }

    void deInit(const std::function<void()>& stop) {
        Logging::log("Shutting down...");

        onDeInit.tick();

        delete resourceManager;

        stop();

        CloseWindow();
    }

    void runBlocking() {
        while (!WindowShouldClose() && !shutdown) {
            isProcessing = true;
            updateAll();
            renderAll();
            isProcessing = false;
            runDeferred();

            gameTime += getScaledDeltaTime();
        }
    }

    void init(const int width, const int height, const char* title, const std::function<void()>& start, const std::function<void()>& stop) {
        resolutionX = width;
        resolutionY = height;

        camera = {
            .offset = {static_cast<float>(width)*0.5f, static_cast<float>(height)*0.5f},
            .target = {0, 0},
            .rotation = 0.0f,
            .zoom = 1.0f,
        };

        resourceManager = new ResourceManager;

        RandomGen::init();

        SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN);
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