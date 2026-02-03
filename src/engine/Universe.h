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

    inline Schedule prepaint;

    inline Schedule onEarlyRender2d; // temporary solution until render ordering is implemented
    inline Schedule onRender2d; // idea... implement a system that will then query the other rendering systems in 'order'
    inline Schedule onLateRender2d;

    inline Schedule onRenderUi;

    inline Schedule onDeInit;

    EntityStorage& getEntityStorage();

    ResourceManager* getResourceManager();
    Input* getInputManager();

    void defer(const std::function<void()>& f);

    void init(int width, int height, const char* title, const std::function<void()>& start, const std::function<void()>& stop);

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

    inline Vec2 getWorldPosition(const Vec2 v) {
        return GetScreenToWorld2D(v, *getCamera());
    }

    inline Vec2 getMouseWorldPosition() {
        return getWorldPosition(GetMousePosition());
    }

    double getGameTime();
    double getTimeScale();

    float getScaledDeltaTime();
} // Universe

struct Timestamp {
    double start;

    explicit Timestamp(const double time) {
        start = time;
    }

    [[nodiscard]] static Timestamp longAgo() {
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
};

#endif //GAME_UNIVERSE_H