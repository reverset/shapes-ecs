#ifndef GAME_STANDARDCOMPONENTS_H
#define GAME_STANDARDCOMPONENTS_H

#include "ecs.h"
#include "vec.h"

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

struct Transient : Component<Transient> {
    COMPONENT_STORAGE(Transient);

    Duration duration{};
    double startTimeSeconds;

    explicit Transient(const Duration dur) {
        this->duration = dur;
        startTimeSeconds = Universe::getGameTime();
    }
};

struct Velocity : Component<Velocity> {
    COMPONENT_STORAGE(Velocity);

    Vec2 velocity = {0, 0};
    float angularVelocity = 0.0f;

    explicit Velocity(const Vec2 linearVel) {
        this->velocity = linearVel;
    }

    explicit Velocity(const float x, float y) {
        velocity = {x, y};
    }

    explicit Velocity(const float x, float y, const float theta) {
        velocity = {x, y};
        angularVelocity = theta;
    }
};

struct ConstantForce : Component<ConstantForce> {
    COMPONENT_STORAGE(ConstantForce);

    Vec2 force = {0, 0};

    explicit ConstantForce(const float xForce, const float yForce) {
        this->force = {xForce, yForce};
    }
};

namespace StandardComponentSystems {
    inline void removeTransient(const Entity e, const Transient& transient) {
        const auto time = Universe::getGameTime();

        if (time > transient.startTimeSeconds + transient.duration.toSeconds()) {
            Universe::defer([e] {
                Universe::getEntityStorage().destroyEntity(e);
            });
        }
    }

    // not automatically registered (for finer control of ordering)
    inline void applyVelocity(const Entity, const Velocity& velocity, Transform2d& trans) {
        trans.position += velocity.velocity * Universe::getScaledDeltaTime();
        trans.rotation += velocity.angularVelocity * Universe::getScaledDeltaTime();
    }

    inline void applyConstantForce(const Entity, const ConstantForce& force, Velocity& vel) {
        vel.velocity += force.force * Universe::getScaledDeltaTime();
    }

    inline void fadeOverTime(const Entity e, const FadeOverTime& time, Sprite& sprite) {
        sprite.tint = Fade(sprite.tint, 
            static_cast<float>(1.0 - time.start.normalizedElapsed(time.fadeTime)));

        if (sprite.tint.a >= 255) {
            Universe::defer([e] {
                auto& store = FadeOverTime::getStoreStatically();
                
                if (store.contains(e)) store.remove(e);
            });
        }
    }

    inline void registerAll() {
        Universe::onUpdate.registerSystem<FadeOverTime, Sprite>(fadeOverTime);
        Universe::onUpdate.registerSystem<ConstantForce, Velocity>(applyConstantForce);
        Universe::onUpdate.registerSystem<Transient>(removeTransient);
    }
}

#endif //GAME_STANDARDCOMPONENTS_H