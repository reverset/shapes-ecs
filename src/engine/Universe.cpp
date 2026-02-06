#include "Universe.h"

#include <vector>
#include <functional>

#include "raylib.h"
#include "inputsys.h"
#include "RandomGen.h"

#include "ecs.h"

namespace Universe {
    std::vector<std::function<void()>> deferredActions;

    RenderTexture2D screenTexture;

    EntityStorage entityStorage;

    EntityStorage& getEntityStorage() {
        return entityStorage;
    }

    Input input;

    bool isProcessing = false;

    int resolutionX, resolutionY;

    float renderResolutionScale = 1.0f;

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

    void updateRenderResolutionScalingFactor() {
        renderResolutionScale = std::min(
            static_cast<float>(GetScreenWidth())/static_cast<float>(screenTexture.texture.width),
            static_cast<float>(GetScreenHeight())/static_cast<float>(screenTexture.texture.height)
        );
    }

    [[nodiscard]] float getResolutionScalingFactor() {
        return renderResolutionScale;
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

        BeginTextureMode(screenTexture);

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

        EndTextureMode();

        BeginDrawing();

        const float scale = getResolutionScalingFactor();

        const auto width = static_cast<float>(GetScreenWidth());
        const auto height = static_cast<float>(GetScreenHeight());

        const auto renderWidth = static_cast<float>(screenTexture.texture.width);
        const auto renderHeight = static_cast<float>(screenTexture.texture.height);

        DrawTexturePro(
            screenTexture.texture,
            {0, 0, renderWidth, -renderHeight},
            {
                (width - renderWidth * scale) * 0.5f,
                (height - renderHeight * scale) * 0.5f,
                renderWidth * scale,
                renderHeight * scale
            },
            {0, 0},
            0.0f,
            WHITE
        );

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

        UnloadRenderTexture(screenTexture);
        CloseWindow();
    }

    void runBlocking() {
        while (!WindowShouldClose() && !shutdown) {
            isProcessing = true;
            updateAll();
            renderAll();
            isProcessing = false;
            runDeferred();

            if (IsWindowResized()) {
                updateRenderResolutionScalingFactor();
            }
            gameTime += getScaledDeltaTime();
        }
    }

    void init(const int width, const int height, const char* title, const std::function<void()>& start, const std::function<void()>& stop, const int renderWidth, const int renderHeight) {
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

        SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN | FLAG_WINDOW_RESIZABLE);

        InitWindow(width, height, title);

        screenTexture = LoadRenderTexture(renderWidth, renderHeight);
        updateRenderResolutionScalingFactor();

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

    int getRenderResolutionWidth() {
        return screenTexture.texture.width;
    }

    int getRenderResolutionHeight() {
        return screenTexture.texture.height;
    }
} // Universe