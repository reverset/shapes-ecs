#ifndef GAME_UNIVERSE_H
#define GAME_UNIVERSE_H

#include <raylib.h>

#include <functional>

#include "util.h"
#include "resource.h"
#include "vec.h"
#include "inputsys.h"
#include "ecs.h"
#include "timer.h"

namespace Universe {
    inline Schedule onUpdate;
    inline Schedule onLateUpdate;

    inline Schedule onEarlyRender2d; // temporary solution until render ordering is implemented
    inline Schedule onRender2d; // idea... implement a system that will then query the other rendering systems in 'order'
    inline Schedule onLateRender2d;

    inline Schedule onRenderUi;

    EntityStorage& getEntityStorage();

    ResourceManager* getResourceManager();
    Input* getInputManager();

    void defer(const std::function<void()>& f);

    void init(int width, int height, const char* title, const std::function<void()>& start, const std::function<void()>& stop);

    int getResolutionX();
    int getResolutionY();

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

    Timestamp() {
        start = Universe::getGameTime();
    }

    [[nodiscard]] bool hasElasped(const Duration dur) const {
        return Universe::getGameTime() > start + dur.toSeconds();
    }
};

#endif //GAME_UNIVERSE_H