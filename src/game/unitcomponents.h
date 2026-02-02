#ifndef GAME_UNITCOMPONENTS_H
#define GAME_UNITCOMPONENTS_H

#include "../engine/ecs.h"

struct Health : Component<Health> {
    COMPONENT_STORAGE(Health);

    std::int32_t maxHealth;
    std::int32_t health;

    explicit Health(const std::int32_t maxHealth) {
        this->maxHealth = maxHealth;
        this->health = maxHealth;
    }

    void damage(const std::int32_t dmg) { // todo damage struct
        health = GameUtil::clamp(health - dmg, 0, maxHealth);
    }
};

#endif //GAME_UNITCOMPONENTS_H