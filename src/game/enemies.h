#ifndef GAME_ENEMIES_H
#define GAME_ENEMIES_H

#include <tuple>

#include "assetstore.h"
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

struct Target: Component<Target> {
    COMPONENT_STORAGE(Target);

    Vec2 target{0, 0};
    Timestamp lastTargetUpdate = Timestamp::longAgo();
    Duration moveInterval = Duration::zero();
};

struct Targeter : Component<Targeter> {
    COMPONENT_STORAGE(Targeter);

    std::vector<Entity> targets;

    explicit Targeter(const std::vector<Entity>&& targets) {
        this->targets = targets;
    }
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

    inline void meanieThink(const Entity e, Meanie& meanie, const Transform2d& trans, Velocity& vel) {
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

            const Vec2 desiredVel = directionToPlayer.normalizeOrZero()*100;
            
            Universe::defer([=] {
                const auto bullet = Spawning::spawnBullet(e, 2, trans.position, desiredVel, 1.0f, BitLayers::PLAYER_LAYER, "bullet", RED);
                Universe::getEntityStorage().insertComponent(bullet, TilemapCollider(8, 8));
            });
        }

        // moving
        if (meanie.lastMoveTime.hasElapsed(meanie.moveInterval)) {
            meanie.lastMoveTime = Timestamp::now();
            meanie.moveInterval = Duration::ofSeconds(RandomGen::randomFloat(0.4f, 2.0f));

            vel.velocity = (playerPos->position + Vec2::randomDirection(20.0f) - trans.position).resize(50.0f);
        }
    }

    inline Entity spawnTarget(const Vec2 pos) {
        const auto texture = AssetStore::getTargetTexture();

        auto sprite = Sprite(texture);
        sprite.tint = RED;

        return Universe::getEntityStorage().makeEntity()
            .addComponent(Target())
            .addComponent(Transform2d(pos, 0.0f, 1.4f))
            .addComponent(std::move(sprite))
            .addComponent(Velocity(0, 0, 5))
            .addComponent(RemoveOnDeath())
            .getEntity();
    }

    inline void targetUpdate(const Entity, Target& circle) {
        if (circle.lastTargetUpdate.hasElapsed(circle.moveInterval)) {

            circle.lastTargetUpdate = Timestamp::now();
            circle.moveInterval = Duration::ofSeconds(RandomGen::randomFloat(0.2f, 2.0f));

            const auto player = ECS::findOneOf<Player, Transform2d>();
            if (!player.has_value()) return;
            const auto playerTrans = std::get<1>(*player);

            circle.target = playerTrans->position;
        }

    }

    inline void updateTargetVel(const Entity, const Target& circle, const Transform2d& trans, Velocity& vel) {
        constexpr float IDEAL_VEL = 200;
        constexpr float ACCEL = 10;

        const auto directionAndMagnitude = (circle.target - trans.position).resize(IDEAL_VEL);
        vel.velocity = vel.velocity.lerp(directionAndMagnitude, ACCEL * Universe::getScaledDeltaTime());
    }

    inline Entity spawnTargeter(const Vec2 pos, const std::size_t targets) {
        std::vector<Entity> t;

        for (std::size_t i = 0; i < targets; i++) {
            const auto target = spawnTarget(pos);
            t.push_back(target);
        }

        auto sprite = Sprite(AssetStore::getPlayerTexture());
        sprite.tint = GREEN;

        return Universe::getEntityStorage().makeEntity()
            .addComponent(Targeter(std::move(t)))
            .addComponent(Transform2d(pos))
            .addComponent(Velocity())
            .addComponent(CollisionRect(16, 16, BitLayers::ENEMY_LAYER, BitLayers::NONE))
            .addComponent(Health(20))
            .addComponent(HealthBar())
            .addComponent(std::move(sprite))
            .addComponent(RemoveOnDeath([](const Entity e) {
                ECS::queryComponentsFor<Targeter>(e, [](const Entity, const Targeter& ta) {
                    auto& es = Universe::getEntityStorage();

                    for (const auto target : ta.targets) {
                        es.destroyEntity(target);
                    }
                });
            }))
            .getEntity();
    }

    inline void registerAll() {
        Universe::onIrregularUpdate
            .registerSystem<Meanie, Transform2d, Velocity>(meanieThink)
            .registerSystem<Target>(targetUpdate);

        Universe::onUpdate
            .registerSystem<Target, Transform2d, Velocity>(updateTargetVel);
    }
}

#endif //GAME_ENEMIES_H