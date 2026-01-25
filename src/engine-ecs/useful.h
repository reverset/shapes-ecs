#ifndef GAME_USEFUL_H
#define GAME_USEFUL_H

#include "../ecs.h"
#include "../timer.h"

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
};

namespace UsefulSystems {
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

    inline void registerAll() {
        Universe::onUpdate.registerSystem<Transient>(removeTransient);
    }
}

#endif //GAME_USEFUL_H