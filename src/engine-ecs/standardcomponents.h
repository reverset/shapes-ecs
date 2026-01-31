#ifndef GAME_STANDARDCOMPONENTS_H
#define GAME_STANDARDCOMPONENTS_H

#include "../ecs.h"
#include "../vec.h"

struct Transform2d : Component<Transform2d> {
    COMPONENT_STORAGE(Transform2d);

    Vec2 position = {0, 0};
    float rotation = 0.0f;
    float scale = 1.0f;

    explicit Transform2d() = default;

    explicit Transform2d(const Vec2& position) : Transform2d(position, 0.0f) {}

    explicit Transform2d(const Vec2& position, const float angle) : Transform2d(position, angle, 1.0f) {}

    explicit Transform2d(const Vec2& position, const float angle, const float scale) {
        this->position = position;
        this->rotation = angle;
        this->scale = scale;
    }
};

struct Sprite : Component<Sprite> {
    COMPONENT_STORAGE(Sprite);

    TextureResource* texture;
    Vec2 offset = {0, 0};
    Color tint = WHITE;

    explicit Sprite(TextureResource* texture) {
        this->texture = texture;
    }

    explicit Sprite(TextureResource* texture, const Vec2 offset) {
        this->texture = texture;
        this->offset = offset;
    }

    explicit Sprite(TextureResource* texture, const Vec2 offset, const Color& tint) {
        this->texture = texture;
        this->offset = offset;
        this->tint = tint;
    }
};

struct FadeOverTime : Component<FadeOverTime> {
    COMPONENT_STORAGE(FadeOverTime);

    Timestamp start = Timestamp::now();
    Duration fadeTime{};

    explicit FadeOverTime(Duration dur) {
        this->fadeTime = dur;
    }
};

namespace StandardComponentSystems {
    void fadeOverTime(const Entity e, const FadeOverTime& time, Sprite& sprite) {
        sprite.tint = Fade(sprite.tint, 
            static_cast<float>(1.0 - time.start.normalizedElapsed(time.fadeTime)));

        if (sprite.tint.a >= 255) {
            Universe::defer([e] {
                auto& store = FadeOverTime::getStoreStatically();
                
                if (store.contains(e)) store.remove(e);
            });
        }
    }

    void registerAll() {
        Universe::onUpdate.registerSystem<FadeOverTime, Sprite>(fadeOverTime);
    }
}

#endif //GAME_STANDARDCOMPONENTS_H