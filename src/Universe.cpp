#include "Universe.h"

#include <iostream>
#include <vector>
#include <functional>

#include "GameObject.h"
#include "raylib.h"
#include "UIObject.h"
#include "inputsys.h"
#include "RandomGen.h"

#include "ecs.h"

namespace Universe {
    // TODO remove
    std::vector<GameObject*> gameObjects; // unique pointers?
    std::vector<std::unique_ptr<UIObject>> uiObjects;

    std::vector<std::function<void()>> deferredActions;

    EntityStorage entityStorage;

    EntityStorage& getEntityStorage() {
        return entityStorage;
    }

    Input input;

    int resolutionX, resolutionY;

    int getResolutionX() {
        return resolutionX;
    }

    int getResolutionY() {
        return resolutionY;
    }

    Camera2D camera;

    bool shutdown = false;
    double gameTime = 0.0;
    double timeScale = 1.0;
    ResourceManager* resourceManager;

    std::vector<GameObject*>* getGameObjects() {
        return &gameObjects;
    }

    std::vector<std::unique_ptr<UIObject>>* getUIObjects() {
        return &uiObjects;
    }

    Input *getInputManager() {
        return &input;
    }

    void defer(const std::function<void()> &f) {
        deferredActions.push_back(f);
    }

    void runDeferred() {
        while (!deferredActions.empty()) {
            auto last = deferredActions.back();
            deferredActions.pop_back();
            last();
        }
    }

    void paintUi() {
        onRenderUi.tick();
        // TODO remove
        for (auto& obj : uiObjects) {
            auto* ptr = obj.get();
            ptr->draw();
        }
    }

    void renderAll() {
        BeginDrawing();
        ClearBackground(DARKGRAY);
        BeginMode2D(camera);

        onRender2d.tick();

        // TODO remove
        for (const auto go : gameObjects) {
            go->render2d();
        }

        EndMode2D();

        paintUi();

        DrawFPS(15, 15);
        EndDrawing();
    }

    void updateAll() {
        onUpdate.tick();

        // TODO remove
        for (const auto game_object : gameObjects) {
            game_object->update();
        }
    }

    void deInit(const std::function<void()>& stop) {
        std::cout << "Shutting down ..." << std::endl;
        for (const auto object : gameObjects) {
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
        resolutionX = width;
        resolutionY = height;

        camera = {
            .offset = {static_cast<float>(width)/2.0f, static_cast<float>(height)/2.0f},
            .target = {0, 0},
            .rotation = 0.0f,
            .zoom = 1.0f,
        };

        resourceManager = new ResourceManager;

        RandomGen::init();
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