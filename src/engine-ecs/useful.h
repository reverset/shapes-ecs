#ifndef GAME_USEFUL_H
#define GAME_USEFUL_H

#include "../ecs.h"
#include "../timer.h"
#include "standardcomponents.h"

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

    inline void applyConstantForce(const Entity, const ConstantForce& force, Velocity& vel) {
        vel.velocity += force.force * Universe::getScaledDeltaTime();
    }

    inline void registerAll() {
        Universe::onUpdate.registerSystem<ConstantForce, Velocity>(applyConstantForce);
        Universe::onUpdate.registerSystem<Transient>(removeTransient);
    }
}



#endif //GAME_USEFUL_H