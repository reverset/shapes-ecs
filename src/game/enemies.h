#ifndef GAME_ENEMIES_H
#define GAME_ENEMIES_H

#include <tuple>

#include "unitcomponents.h"
#include "../engine/ecs.h"
#include "../engine/standardcomponents.h"
#include "../engine/Universe.h"
#include "../engine/vec.h"
#include "../engine/tilemap.h"

struct Meanie : Component<Meanie> {
    COMPONENT_STORAGE(Meanie);

    Timestamp lastShootTime = Timestamp::longAgo();

    Timestamp lastMoveTime = Timestamp::longAgo();
    Duration moveInterval = Duration::zero();
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

    inline void meanieThink(const Entity e, Meanie& meanie, Transform2d& trans) {
        constexpr auto shootCooldownTime = Duration::ofSeconds(0.5);

        const auto playerQuery = ECS::findOneOf<Player, Transform2d>();
        if (!playerQuery.has_value()) {
            Logging::logWarn("Player is missing!");
            return;
        }

        // shooting
        if (meanie.lastShootTime.hasElapsed(shootCooldownTime)) {
            meanie.lastShootTime = Timestamp::now();

            const Transform2d* playerPos = std::get<1>(*playerQuery);
            const Vec2 vel = (playerPos->position - trans.position).normalizeOrZero()*100;
            
            Universe::defer([=] {
                const auto bullet = Spawning::spawnBullet(e, 2, trans.position, vel, 1.0f, BitLayers::PLAYER_LAYER, "bullet", RED);
                Universe::getEntityStorage().insertComponent(bullet, TilemapCollider(8, 8));
            });
        }

        // trans.position = trans.position.lerp(playerPos->position, Universe::getScaledDeltaTime());
    }

    inline void registerAll() {
        Universe::onUpdate.registerSystem<Meanie, Transform2d>(meanieThink);
    }
}

#endif //GAME_ENEMIES_H