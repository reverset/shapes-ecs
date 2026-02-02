#ifndef GAME_STANDARDCOMPONENTS_H
#define GAME_STANDARDCOMPONENTS_H

#include "ecs.h"
#include "event.h"
#include "vec.h"

#include "resource.h"
#include "Universe.h"

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

    explicit FadeOverTime(const Duration dur) {
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


struct CollisionRect : Component<CollisionRect> {
    COMPONENT_STORAGE(CollisionRect);

    Vec2 dimensions = {0, 0};
    std::uint32_t layer = 0;
    std::uint32_t mask = 0;

    explicit CollisionRect(const float width, const float height, const std::uint32_t layer, const std::uint32_t mask) {
        this->dimensions = {width, height};
        this->layer = layer;
        this->mask = mask;
    }

    explicit CollisionRect(const float width, const float height) : CollisionRect(width, height, 0, 0) {}

    CollisionRect& setLayerAt(const std::uint32_t l, const bool val) {
        if (val) layer |= (1u << l);
        else layer &= ~(1u << l);

        return *this;
    }

    CollisionRect& setMaskAt(const std::uint32_t l, const bool val) {
        if (val) mask |= (1u << l);
        else mask &= ~(1u << l);

        return *this;
    }

    [[nodiscard]] constexpr bool layerOverlapWithMask(const std::uint32_t m) const {
        return (layer & m) != 0;
    }

    [[nodiscard]] constexpr bool isOnLayer(const std::uint32_t l) const {
        return (layer & (1u << l)) != 0;
    }

    [[nodiscard]] constexpr bool checkMask(const std::uint32_t l) const {
        return (mask & (1u << l)) != 0;
    }

    [[nodiscard]] constexpr Rectangle makeRect(const Vec2 pos) const {
        return {
            pos.x - dimensions.x*0.5f, // center
            pos.y - dimensions.y*0.5f,
            dimensions.x,
            dimensions.y,
        };
    }
};

struct ColliderOverlapEvent : Event<ColliderOverlapEvent> {
    EVENT_STORAGE(ColliderOverlapEvent);

    Entity a;
    Entity b;

    CollisionRect* cA;
    CollisionRect* cB;
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

    inline void debugDrawCollisionRects(const Entity, const CollisionRect& collisionRect, const Transform2d& trans) {
        DrawRectangleLinesEx(collisionRect.makeRect(trans.position), 2.0f, RED);
    }

    inline void checkColliderOverlap(const Entity a, CollisionRect& cRect, const Transform2d& trans) {
        // TODO, obviously dont check collision with EVERYTHING!!! (perhaps use spatial hashing again? octrees?)
        ECS::query<CollisionRect, Transform2d>([&](const Entity b, CollisionRect& rect2, const Transform2d& trans2) {
            if (a.id == b.id) return;
            if (!rect2.layerOverlapWithMask(cRect.mask)) return; // perhaps layers and masks should just be components.

            if (CheckCollisionRecs(cRect.makeRect(trans.position), rect2.makeRect(trans2.position))) {
                ColliderOverlapEvent evt = {
                    .a = a,
                    .b = b,
                    .cA = &cRect,
                    .cB = &rect2,
                };
                evt.send();
            }
        });
    }

    inline void registerAll() {
        Universe::onUpdate.registerSystem<FadeOverTime, Sprite>(fadeOverTime);
        Universe::onUpdate.registerSystem<ConstantForce, Velocity>(applyConstantForce);
        Universe::onUpdate.registerSystem<Transient>(removeTransient);
        Universe::onUpdate.registerSystem<CollisionRect, Transform2d>(checkColliderOverlap);
    }

    inline void enableDebugRendering() {
        Universe::onLateRender2d.registerSystem<CollisionRect, Transform2d>(debugDrawCollisionRects);
    }
}

namespace RenderingSystems {
    inline void renderSprites(const Entity, const Sprite& sprite, const Transform2d& trans) {
        sprite.texture->renderEx(trans.position, sprite.offset, trans.rotation, trans.scale, sprite.tint);
    }

    inline void registerAll() {
        Universe::onRender2d.registerSystem<Sprite, Transform2d>(renderSprites);
    }
}

#endif //GAME_STANDARDCOMPONENTS_H