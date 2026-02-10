#ifndef GAME_UI_H
#define GAME_UI_H


#include "vec.h"
#include "Universe.h"

namespace UI {
    [[nodiscard]] Vec2 getRenderDimensions() {
        return { Universe::getRenderResolutionWidth(), Universe::getRenderResolutionHeight() };
    }


    [[nodiscard]] Vec2 percentFromBLCorner(const Vec2 offset) {
        ASSERT(offset.x <= 1 && offset.y <= 1 && offset.x >= 0 && offset.y >= 0, "offset's components are not within 0..=1.");
        return Vec2{ offset.x, 1.0 - offset.y } * getRenderDimensions();
    }
}

#endif