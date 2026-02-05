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
        const auto sprite = AssetStore::getMeanieTexture();
        return Universe::getEntityStorage().makeEntity()
            .addComponent(Meanie())
            .addComponent(Transform2d(pos))
            .addComponent(Sprite(sprite))
            .addComponent(TilemapCollider(16, 16))
            .addComponent(CollisionRect(16, 16, BitLayers::ENEMY_LAYER, BitLayers::NONE))
            .addComponent(Health(50))
            .addComponent(HealthBar())
            .addComponent(Velocity())
            .addComponent(RemoveOnDeath())
            .getEntity();
    }

    inline void meanieThink(const Entity e, Meanie& meanie, Transform2d& trans, Velocity& vel) {
        constexpr auto shootCooldownTime = Duration::ofSeconds(0.5);

        const auto playerQuery = ECS::findOneOf<Player, Transform2d>();
        if (!playerQuery.has_value()) {
            Logging::logWarn("Player is missing!");
            return;
        }
    
        const Transform2d* playerPos = std::get<1>(*playerQuery);
        const auto directionToPlayer = (playerPos->position - trans.position).normalizeOrZero();

        // shooting
        if (meanie.lastShootTime.hasElapsed(shootCooldownTime)) {
            meanie.lastShootTime = Timestamp::now();

            const Vec2 vel = directionToPlayer.normalizeOrZero()*100;
            
            Universe::defer([=] {
                const auto bullet = Spawning::spawnBullet(e, 2, trans.position, vel, 1.0f, BitLayers::PLAYER_LAYER, "bullet", RED);
                Universe::getEntityStorage().insertComponent(bullet, TilemapCollider(8, 8));
            });
        }

        // moving
        if (meanie.lastMoveTime.hasElapsed(meanie.moveInterval)) {
            meanie.lastMoveTime = Timestamp::now();
            meanie.moveInterval = Duration::ofSeconds(RandomGen::randomFloat(0.2f, 2.0f));

            vel.velocity = (playerPos->position + Vec2::randomDirection(20.0f) - trans.position).resize(50.0f);
        }
    }

    inline void registerAll() {
        Universe::onUpdate.registerSystem<Meanie, Transform2d, Velocity>(meanieThink);
    }
}

#endif //GAME_ENEMIES_H