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

namespace UsefulSystems {
    inline void removeTransient(const Entity e, const Transient& transient) {
        const auto time = Universe::getGameTime();

        if (time > transient.startTimeSeconds + transient.duration.toSeconds()) {
            Universe::defer([e] {
                Universe::getEntityStorage().destroyEntity(e);
            });
        }
    }

    inline void registerAll() {
        Universe::onUpdate.registerSystem<Transient>(removeTransient);
    }
}

#endif //GAME_USEFUL_H