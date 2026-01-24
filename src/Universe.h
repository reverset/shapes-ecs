#ifndef GAME_UNIVERSE_H
#define GAME_UNIVERSE_H

#include <raylib.h>

#include <functional>

#include "util.h"
#include "GameObject.h"
#include "resource.h"
#include "UIObject.h"
#include "vec.h"
#include "inputsys.h"
#include "ecs.h"

namespace Universe {
    inline Schedule onUpdate;
    inline Schedule onRender2d;
    inline Schedule onRenderUi;

    ResourceManager* getResourceManager();
    Input* getInputManager();

    void defer(const std::function<void()>& f);

    void init(int width, int height, const char* title, const std::function<void()>& start, const std::function<void()>& stop);

    int getResolutionX();
    int getResolutionY();

    std::vector<std::unique_ptr<UIObject>>* getUIObjects();

    std::vector<GameObject*>* getGameObjects();

    template <class T>
        requires std::derived_from<T, GameObject>
    T* instantiate(T* obj) {
        auto gos = getGameObjects();
        // gos->push_back(obj);

        const std::function<int(GameObject* const &, GameObject* const &)> cmp = [](GameObject* const & a, GameObject* const & b) {
            return b->processLayer - a->processLayer;
        };

        GameUtil::insertSorted<GameObject*>(gos, obj, cmp);

        obj->ready();

        return obj;
    }

    template <class T>
        requires std::derived_from<T, GameObject>
    T* findByName(const std::string& name) {
        for (auto obj : *getGameObjects()) {
            if (obj->name == name) {
                return dynamic_cast<T*>(obj);
            }
        }
        return nullptr;
    }

    template <class T>
        requires std::derived_from<T, UIObject>
    void addUiElement(std::unique_ptr<T>&& ui) {
        auto uiEles = getUIObjects();

        GameUtil::insertSorted<std::unique_ptr<UIObject>>(uiEles, std::move(ui), [](const std::unique_ptr<UIObject>& a, const std::unique_ptr<UIObject>& b) {
            return b->layer - a->layer;
        });
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