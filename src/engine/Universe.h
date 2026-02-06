#ifndef GAME_UNIVERSE_H
#define GAME_UNIVERSE_H

#include <raylib.h>

#include <functional>

#include "resource.h"
#include "vec.h"
#include "inputsys.h"
#include "ecs.h"
#include "timer.h"
#include "util.h"

namespace Universe {
    inline Schedule onUpdate;
    inline Schedule onLateUpdate;

    inline auto onIrregularUpdate = IrregularSchedule(5, 0.2);

    inline Schedule prepaint;

    inline Schedule onEarlyRender2d; // temporary solution until render ordering is implemented
    inline Schedule onRender2d; // idea... implement a system that will then query the other rendering systems in 'order'
    inline Schedule onLateRender2d;

    inline Schedule onRenderUi;

    inline Schedule onFinalFrameUpdate;
    inline Schedule onDeInit;

    EntityStorage& getEntityStorage();

    ResourceManager* getResourceManager();
    Input* getInputManager();

    void defer(const std::function<void()>& f);

    void init(int width, int height, const char* title, const std::function<void()>& start, const std::function<void()>& stop, int renderWidth = 640, int renderHeight = 360);

    int getResolutionX();
    int getResolutionY();

    bool areEntitiesBusy();

    inline float getAxisInput(const KeyboardKey neg, const KeyboardKey pos) {
        return (IsKeyDown(neg) ? -1.0f : 0.0f)
            + (IsKeyDown(pos) ? 1.0f : 0.0f);
    }

    inline Vec2 getVectorInput(
        const KeyboardKey negX, const KeyboardKey posX,
        const KeyboardKey negY, const KeyboardKey posY) {
       return Vec2(getAxisInput(negX, posX), getAxisInput(negY, posY)).normalizeOrZero();
    }

    Camera2D* getCamera();

    [[nodiscard]] int getRenderResolutionWidth();
    [[nodiscard]] int getRenderResolutionHeight();
    [[nodiscard]] float getResolutionScalingFactor();

    inline Vec2 getVirtualScreenPosition(const Vec2 screenPos) {
        const float scale = getResolutionScalingFactor();

        const auto width = static_cast<float>(GetScreenWidth());
        const auto height = static_cast<float>(GetScreenHeight());

        const auto renderWidth = static_cast<float>(getRenderResolutionWidth());
        const auto renderHeight = static_cast<float>(getRenderResolutionHeight());

        auto pos = Vec2{
            (screenPos.x - (width - (renderWidth * scale)) * 0.5f) / scale,
            (screenPos.y - (height - (renderHeight * scale)) * 0.5f) / scale,
        };

        pos = pos.clamp(Vec2::zero(), Vec2(renderWidth, renderHeight));
        return pos;
    }

    inline Vec2 getWorldPosition(const Vec2 v) {
        return GetScreenToWorld2D(getVirtualScreenPosition(v), *getCamera());
    }

    inline Vec2 getMouseWorldPosition() {
        return getWorldPosition(GetMousePosition());
    }

    double getGameTime();
    double getTimeScale();

    float getScaledDeltaTime();

    [[nodiscard]] inline int getWindowWidth() {
        return GetScreenWidth();
    }

    [[nodiscard]] inline int getWindowHeight() {
        return GetScreenHeight();
    }
} // Universe

struct Timestamp {
    double start;

    explicit Timestamp(const double time) {
        start = time;
    }

    [[nodiscard]] constexpr static Timestamp longAgo() {
        return Timestamp(
            std::numeric_limits<double>::lowest()
        );
    }

    [[nodiscard]] static Timestamp now() {
        return Timestamp(Universe::getGameTime());
    }

    [[nodiscard]] bool hasElapsed(const Duration dur) const {
        return Universe::getGameTime() > start + dur.toSeconds();
    }

    [[nodiscard]] double normalizedElapsed(Duration max) const {
        return GameUtil::clamp(
            (Universe::getGameTime() - start) / max.toSeconds(), 
            0.0,
            1.0
        );
    }

    [[nodiscard]] Timestamp shift(const Duration dur) const {
        return Timestamp (start + dur.toSeconds());
    }
};

#endif //GAME_UNIVERSE_H