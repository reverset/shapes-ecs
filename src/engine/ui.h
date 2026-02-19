#ifndef GAME_UI_H
#define GAME_UI_H


#include "vec.h"
#include "Universe.h"
#include "ecs.h"

namespace UI {
    [[nodiscard]] inline Vec2 getRenderDimensions() {
        return { static_cast<float>(Universe::getRenderResolutionWidth()), static_cast<float>(Universe::getRenderResolutionHeight()) };
    }


    [[nodiscard]] inline Vec2 percentFromBLCorner(const Vec2 offset) {
        ASSERT(offset.x <= 1 && offset.y <= 1 && offset.x >= 0 && offset.y >= 0, "offset's components are not within 0..=1.");
        return Vec2{ offset.x, 1.0f - offset.y } * getRenderDimensions();
    }

    class Text : Component<Text> {
    public:
        COMPONENT_STORAGE(Text);

        std::string text;

        explicit Text(const std::string& text) : text(text) {}
    };
}

#endif