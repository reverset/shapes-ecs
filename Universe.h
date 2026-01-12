#ifndef GAME_UNIVERSE_H
#define GAME_UNIVERSE_H

#include <raylib.h>

#include <functional>

#include "GameObject.h"
#include "resource.h"
#include "vec.h"

namespace Universe {
    ResourceManager* getResourceManager();

    void defer(const std::function<void()>& f);

    void init(int width, int height, const char* title, const std::function<void()>& start, const std::function<void()>& stop);

    std::vector<GameObject*>& getGameObjects();

    template <class T>
        requires std::derived_from<T, GameObject>
    T* instantiate(T* obj) {
        getGameObjects().push_back(obj);
        obj->ready();

        return obj;
    }

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

#endif //GAME_UNIVERSE_H