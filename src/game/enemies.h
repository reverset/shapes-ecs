#ifndef GAME_ENEMIES_H
#define GAME_ENEMIES_H

#include "unitcomponents.h"
#include "../engine/ecs.h"
#include "../engine/standardcomponents.h"
#include "../engine/Universe.h"
#include "../engine/vec.h"

struct Meanie : Component<Meanie> {
    COMPONENT_STORAGE(Meanie);


};

namespace Enemies {
    inline Entity spawnMeanie(const Vec2 pos) {
        const auto sprite = *Universe::getResourceManager()->getResource<TextureResource>("meanie");
        return Universe::getEntityStorage().makeEntity()
            .addComponent(Meanie())
            .addComponent(Transform2d(pos))
            .addComponent(Sprite(sprite))
            .addComponent(CollisionRect(16, 16, BitLayers::ENEMY_LAYER, BitLayers::NONE))
            .addComponent(Health(50))
            .addComponent(HealthBar())
            .getEntity();
    }
}

#endif //GAME_ENEMIES_H