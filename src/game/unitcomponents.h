#ifndef GAME_UNITCOMPONENTS_H
#define GAME_UNITCOMPONENTS_H

#include "raylib.h"

#include "../engine/Universe.h"
#include "../engine/ecs.h"
#include "../engine/standardcomponents.h"
#include "../engine/logging.h"
#include "../engine/util.h"

struct Weapon {
    Timestamp lastFiredTimestamp = Timestamp::longAgo();
    Duration cooldownTime{};

    explicit Weapon(const Duration cooldownTime) {
        this->cooldownTime = cooldownTime;
    }
};

struct Player : Component<Player> {
    COMPONENT_STORAGE(Player);

    Weapon heldWeapon{Duration::ofSeconds(0.2)};
};

struct Bullet : Component<Bullet> {
    COMPONENT_STORAGE(Bullet);

    Entity attacker{};
    std::uint32_t baseDamage;

    Bullet() = delete;

    explicit Bullet(const Entity attacker, const std::uint32_t baseDamage) {
        this->attacker = attacker;
        this->baseDamage = baseDamage;
    }
};

struct Health : Component<Health> {
    COMPONENT_STORAGE(Health);

    std::int32_t maxHealth;
    std::int32_t health;

    Timestamp lastDamage = Timestamp::longAgo();

    explicit Health(const std::int32_t maxHealth) {
        this->maxHealth = maxHealth;
        this->health = maxHealth;
    }

    void damage(const std::uint32_t dmg) { // todo damage struct
        health = GameUtil::clamp(health - static_cast<std::int32_t>(dmg), 0, maxHealth);
        lastDamage = Timestamp::now();
    }

    [[nodiscard]] bool isDead() const {
        return health <= 0;
    }

    [[nodiscard]] float getHealthNormalized() const {
        return static_cast<float>(health) / static_cast<float>(maxHealth);
    }
};

MARKER_COMPONENT(RemoveOnDeath);
MARKER_COMPONENT(DeathMarker);

struct HealthBar : Component<HealthBar> {
    COMPONENT_STORAGE(HealthBar);

    float width = 30.0f;
    float height = 5.0f;
    float yOffset = -20.0f;

    float damageBarProgress = 1.0f;
};


// this event should only be invoked in a deferred context! (at the end of the frame)
struct DeathEvent : Event<DeathEvent> {
    EVENT_STORAGE(DeathEvent);

    Entity victim;
};

namespace Spawning {
    inline Entity spawnBullet(const Entity attacker, const std::uint32_t baseDamage, const Vec2 pos, const Vec2 vel, const float hitboxScale, const BitLayers::Type mask, const std::string& spriteName = "bullet", const Color tint = WHITE) {
        constexpr auto lifetime = Duration::ofSeconds(2.0);
        constexpr auto beginFadeOffset = Duration::ofSeconds(1.0);
        constexpr auto fadeTime = lifetime - beginFadeOffset;
        
        const auto sprite = *Universe::getResourceManager()->getResource<TextureResource>(spriteName);

        auto& store = Universe::getEntityStorage();
        return store.makeEntity()
            .addComponent(Sprite(sprite, tint))
            .addComponent(Transform2d(pos, vel.toAngle(), 0.8f))
            .addComponent(Bullet(attacker, baseDamage))
            .addComponent(Velocity(vel))
            .addComponent(Transient(lifetime))
            .addComponent(FadeOverTime(fadeTime, beginFadeOffset))
            .addComponent(CollisionRect(16 * hitboxScale, 16 * hitboxScale, BitLayers::NONE, mask))
            .getEntity();
    }
}

namespace UnitComponents {
    inline void renderHealthBars(const Entity, HealthBar& bar, const Health& health, const Transform2d& trans) {
        constexpr auto damageCatchupWithHealthDelay = Duration::ofSeconds(0.5);
        constexpr float damageCatchupSpeed = 2.0f;
        
        // background setup for healthbar
        const float x = trans.position.x - (bar.width * 0.5f);
        const float y = trans.position.y + bar.yOffset;
        
        Rectangle background = {
            x, y,
            bar.width, bar.height,
        };

        // the gold part that slowly catches up to the actual health
        const float normalizedHealth = health.getHealthNormalized();

        if (health.lastDamage.hasElapsed(damageCatchupWithHealthDelay)) {
            bar.damageBarProgress = GameUtil::moveTowards(bar.damageBarProgress, normalizedHealth, damageCatchupSpeed * Universe::getScaledDeltaTime());
        }
        
        const float damageWidth = bar.damageBarProgress * bar.width;
        const float damageX = trans.position.x - (damageWidth * 0.5f) - (bar.width - damageWidth) * 0.5f;

        Rectangle damageBar = {
            damageX, y,
            damageWidth, bar.height,
        };

        // and finally the actually red health bar
        const float healthWidth = normalizedHealth * bar.width;
        const float healthX = trans.position.x - (healthWidth * 0.5f) - (bar.width - healthWidth) * 0.5f;

        Rectangle healthBar = {
            healthX, y,
            healthWidth, bar.height,
        };

        DrawRectangleRounded(background, 0.8, 4, GRAY);
        DrawRectangleRounded(damageBar, 0.8, 4, GOLD);
        DrawRectangleRounded(healthBar, 0.8, 4, RED);
    }

    inline void deathEventEmitter(const Entity e, const Health& health) {
        if (ECS::hasComponents<DeathMarker>(e)) return; // TODO, improve queries for systems to have something like Not<DeathMarker>

        if (health.isDead()) {
            Universe::getEntityStorage()
                .insertComponent<DeathMarker>(e, DeathMarker());


            Universe::defer([e] {
                DeathEvent evt = {
                    .victim = e,
                };
                evt.send();
            });
        }
    }

    inline void removeDead(const Entity e, const RemoveOnDeath&, const DeathMarker&) {
        Universe::getEntityStorage()
            .destroyEntity(e);
    }

    inline void registerAll() {
        Universe::onUpdate
            .registerSystem<Health>(deathEventEmitter);

        Universe::onLateRender2d
            .registerSystem<HealthBar, Health, Transform2d>(renderHealthBars);

        Universe::onFinalFrameUpdate
            .registerSystem<RemoveOnDeath, DeathMarker>(removeDead);
    }
}

#endif //GAME_UNITCOMPONENTS_H