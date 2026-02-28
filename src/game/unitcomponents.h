#ifndef GAME_UNITCOMPONENTS_H
#define GAME_UNITCOMPONENTS_H

#include <utility>

#include "assetstore.h"
#include "BitLayers.h"
#include "particles.h"
#include "raylib.h"

#include "../engine/Universe.h"
#include "../engine/ecs.h"
#include "../engine/standardcomponents.h"
#include "../engine/logging.h"
#include "../engine/util.h"

struct PopupText : Component<PopupText> {
    COMPONENT_STORAGE(PopupText);

    FontConfig font;
    std::string text;
    Timestamp spawnTime = Timestamp::now();

    explicit PopupText(const FontConfig& font, std::string text) : font(font), text(std::move(text)) {}
};

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
    Timestamp lastHeal = Timestamp::longAgo();

    explicit Health(const std::int32_t maxHealth) {
        this->maxHealth = maxHealth;
        this->health = maxHealth;
    }

    void heal(const std::uint32_t healing, const Vec2 where = Vec2::nan()) {
        health = GameUtil::clamp(health + static_cast<std::int32_t>(healing), 0, maxHealth);
        lastHeal = Timestamp::now();
    }

    void damage(const std::uint32_t dmg, const Vec2 where = Vec2::nan()) { // todo damage struct
        health = GameUtil::clamp(health - static_cast<std::int32_t>(dmg), 0, maxHealth);
        lastDamage = Timestamp::now();

        if (!where.isNan()) { // make function for this
            Universe::defer([dmg, where] {
                const std::string text = std::format("{}!", dmg); // TODO use std::format for logger!

                constexpr auto lifetime = Duration::ofSeconds(2.0);
                constexpr auto fadeoutOffset = Duration::ofSeconds(1.0);
                constexpr auto fadeout = Duration::ofSeconds(1.0);

                const auto font = FontConfig(AssetStore::getJetbrainsMonoRegular(), 12, 1, true);

                Universe::getEntityStorage().makeEntity()
                    .addComponent(PopupText(font, text))
                    .addComponent(Transient(lifetime))
                    .addComponent(FadeOverTime(fadeout, fadeoutOffset))
                    .addComponent(Transform2d(where));
            });
        }
    }

    [[nodiscard]] bool isDead() const {
        return health <= 0;
    }

    [[nodiscard]] float getHealthNormalized() const {
        return static_cast<float>(health) / static_cast<float>(maxHealth);
    }
};

struct RemoveOnDeath : Component<RemoveOnDeath> {
    COMPONENT_STORAGE(RemoveOnDeath);

    std::function<void(Entity)> onDeath = nullptr;

    explicit RemoveOnDeath() = default;

    explicit RemoveOnDeath(const std::function<void(Entity)>& onDeath) {
        this->onDeath = onDeath;
    }
};

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

struct DamageVolume : Component<DamageVolume> {
    COMPONENT_STORAGE(DamageVolume);

    Vec2 dimensions;
    BitLayers::Type mask;
    std::uint32_t damage;
    Duration hitInterval;
    Timestamp lastHit = Timestamp::longAgo();

    explicit DamageVolume(
        const BitLayers::Type mask,
        const std::uint32_t damage,
        const Vec2 dimensions,
        const Duration hitInterval
        ) : dimensions(dimensions), mask(mask), damage(damage), hitInterval(hitInterval) {}
};

struct IncomingHealthModifyingVolume : Component<IncomingHealthModifyingVolume> {
    COMPONENT_STORAGE(IncomingHealthModifyingVolume);

    Vec2 dimensions;
    BitLayers::Type layer;

    explicit IncomingHealthModifyingVolume(const BitLayers::Type layer, const Vec2 dimensions) : dimensions(dimensions), layer(layer) {}
};

struct HealingVolume : Component<HealingVolume> {
    COMPONENT_STORAGE(HealingVolume);

    Vec2 dimensions;
    BitLayers::Type mask;
    std::uint32_t healing;
    Duration healInterval;
    Timestamp lastHeal = Timestamp::longAgo();

    explicit HealingVolume(
        const BitLayers::Type mask,
        const std::uint32_t healing,
        const Vec2 dimensions,
        const Duration hitInterval
        ) : dimensions(dimensions), mask(mask), healing(healing), healInterval(hitInterval) {}
};

struct HealingHeart : Component<HealingHeart> {
    COMPONENT_STORAGE(HealingHeart);   
};

struct OnDamageDealtByVolume : Event<OnDamageDealtByVolume> {
    EVENT_STORAGE(OnDamageDealtByVolume);

    Entity attacker;
    Entity victim;
};

struct OnHealingDealtByVolume : Event<OnHealingDealtByVolume> {
    EVENT_STORAGE(OnHealingDealtByVolume);

    Entity doctor;
    Entity patient;
};

namespace Spawning {
    inline Entity spawnHealingHeart(const std::uint32_t healing, const Vec2 pos, const Vec2 vel) {
        const auto texture = AssetStore::getHealingHeart();
        
        auto sprite = Sprite(texture)
            .withShader(AssetStore::getHealingHeartShader());

        return Universe::getEntityStorage().makeEntity()
            .addComponent(HealingHeart())
            .addComponent(HealingVolume(BitLayers::PLAYER_LAYER, healing, {16, 16}, Duration::ofSeconds(1.0)))
            .addComponent(std::move(sprite))
            .addComponent(AutoShaderGameTimeUpdate())
            .addComponent(RenderLayer4())
            .addComponent(Transient(Duration::ofSeconds(10.0)))
            .addComponent(Transform2d(pos, 0.0f, 1.0f))
            .getEntity();
    }

    inline Entity spawnBullet(const Entity attacker, const std::uint32_t baseDamage, const Vec2 pos, const Vec2 vel, const float hitboxScale, const BitLayers::Type mask, const std::string& spriteName = "bullet", const Color tint = WHITE) {
        constexpr auto lifetime = Duration::ofSeconds(2.0);
        constexpr auto beginFadeOffset = Duration::ofSeconds(1.0);
        constexpr auto fadeTime = lifetime - beginFadeOffset;
        
        const auto texture = *Universe::getResourceManager()->getResource<TextureResource>(spriteName);

        // const auto outlineShader = AssetStore::getOutlineShader();
        // outlineShader->setField("outlineSize", 2.0f);
        // outlineShader->setField("textureSize", Vec2{16, 16});
        // outlineShader->setField("outlineColor", BLUE);

        auto sprite = Sprite(texture, tint);
            // .withShader(outlineShader);

        auto& store = Universe::getEntityStorage();
        return store.makeEntity()
            .addComponent(std::move(sprite))
            .addComponent(Transform2d(pos, vel.toAngle(), 0.8f))
            .addComponent(Bullet(attacker, baseDamage))
            .addComponent(Velocity(vel))
            .addComponent(Transient(lifetime))
            .addComponent(RenderLayer1())
            .addComponent(FadeOverTime(fadeTime, beginFadeOffset))
            .addComponent(DamageVolume(mask, baseDamage, {16 * hitboxScale, 16 *  hitboxScale}, Duration::ofSeconds(1.0)))
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

        const Rectangle damageBar = {
            damageX, y,
            damageWidth, bar.height,
        };

        // and finally the actually red health bar
        const float healthWidth = normalizedHealth * bar.width;
        const float healthX = trans.position.x - (healthWidth * 0.5f) - (bar.width - healthWidth) * 0.5f;

        const Rectangle healthBar = {
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

    inline void removeDead(const Entity e, const RemoveOnDeath& rd, const DeathMarker&) {
        if (rd.onDeath != nullptr) {
            rd.onDeath(e);
        }

        Universe::getEntityStorage()
            .destroyEntity(e);
    }

    inline void checkForDamageViaVolumes(const Entity e1, DamageVolume& damageVolume, const Transform2d& trans) {
        if (!damageVolume.lastHit.hasElapsed(damageVolume.hitInterval)) return;

        ECS::query<IncomingHealthModifyingVolume, Transform2d, Health>([&](const Entity e2, const IncomingHealthModifyingVolume& damageReceiver, const Transform2d& trans2, Health& health) {
            if (!BitLayers::checkMask(damageVolume.mask, damageReceiver.layer)) return;

            const auto rect1 = GameUtil::centeredRect(
                trans.position.x, trans.position.y,
                damageVolume.dimensions.x, damageVolume.dimensions.y);
            const auto rect2 = GameUtil::centeredRect(
                trans2.position.x, trans2.position.y,
                damageReceiver.dimensions.x, damageReceiver.dimensions.y);

            if (CheckCollisionRecs(rect1, rect2)) {
                damageVolume.lastHit = Timestamp::now();
                health.damage(damageVolume.damage, trans.position);

                OnDamageDealtByVolume evt = {
                    .attacker = e1,
                    .victim = e2,
                };

                evt.send();
            }
        });
    }

    // duplicate code FIXME
    inline void checkForHealingViaVolumes(const Entity e1, HealingVolume& healingVolume, const Transform2d& trans) {
        if (!healingVolume.lastHeal.hasElapsed(healingVolume.healInterval)) return;

        ECS::query<IncomingHealthModifyingVolume, Transform2d, Health>([&](const Entity e2, const IncomingHealthModifyingVolume& patient, const Transform2d& trans2, Health& health) {
            if (!BitLayers::checkMask(healingVolume.mask, patient.layer)) return;

            const auto rect1 = GameUtil::centeredRect(
                trans.position.x, trans.position.y,
                healingVolume.dimensions.x, healingVolume.dimensions.y);
            const auto rect2 = GameUtil::centeredRect(
                trans2.position.x, trans2.position.y,
                patient.dimensions.x, patient.dimensions.y);

            if (CheckCollisionRecs(rect1, rect2)) {
                healingVolume.lastHeal = Timestamp::now();
                health.heal(healingVolume.healing);

                OnHealingDealtByVolume evt = {
                    .doctor = e1,
                    .patient = e2,
                };

                evt.send();
            }
        });
    }

    inline void debugVolumes() {
        ECS::query<IncomingHealthModifyingVolume, Transform2d>([](const Entity, const IncomingHealthModifyingVolume& damageReceiver, const Transform2d& trans) {
            const auto rect1 = GameUtil::centeredRect(
               trans.position.x, trans.position.y,
               damageReceiver.dimensions.x, damageReceiver.dimensions.y);

            DrawRectangleRoundedLinesEx(rect1, 0.2, 4, 2, PINK);
        });

        ECS::query<DamageVolume, Transform2d>([](const Entity, const DamageVolume& damageVolume, const Transform2d& trans) {
            const auto rect1 = GameUtil::centeredRect(
               trans.position.x, trans.position.y,
               damageVolume.dimensions.x, damageVolume.dimensions.y);

            DrawRectangleRoundedLinesEx(rect1, 0.2, 4, 2, RED);
        });
    }

    inline void drawPopupText(const Entity, PopupText& text, const Transform2d& trans) {
        // perhaps TextFont should store font size and spacing?
        // TODO use fade out time!
        constexpr auto dur = Duration::ofSeconds(1.0);
        const auto norm = text.spawnTime.normalizedElapsed(dur);

        const auto fontSize = text.font.fontSize * (0.5 / (norm+0.1));
        // text.font->render(text.text.c_str(), trans.position, static_cast<float>(fontSize), 1, WHITE, true);
        text.font.render(text.text.c_str(), trans.position, WHITE, static_cast<float>(fontSize));
    }

    inline void registerAll() {
        Universe::onUpdate
            .registerSystem<DamageVolume, Transform2d>(checkForDamageViaVolumes)
            .registerSystem<HealingVolume, Transform2d>(checkForHealingViaVolumes)
            .registerSystem<Health>(deathEventEmitter);

        const auto drawStuff = [] {
            ECS::query<HealthBar, Health, Transform2d>(renderHealthBars);
            ECS::query<PopupText, Transform2d>(drawPopupText);
        };

        Universe::onLateRender2d
            .registerCallable(drawStuff);
            // .registerCallable(debugVolumes)
            // .registerSystem<PopupText, Transform2d>(drawPopupText)
            // .registerSystem<HealthBar, Health, Transform2d>(renderHealthBars);

        Universe::onFinalFrameUpdate
            .registerSystem<RemoveOnDeath, DeathMarker>(removeDead);

        OnHealingDealtByVolume::listen([](const OnHealingDealtByVolume& volume) {
            ECS::queryComponentsFor<HealingHeart, Transform2d>(volume.doctor, [](const Entity e, const HealingHeart&, const Transform2d& trans) {
                Particles::sparkle(trans.position, 14, PINK);

                Universe::defer([e] {
                    Universe::getEntityStorage().destroyEntity(e);
                });
            });
        });
    }
}

#endif //GAME_UNITCOMPONENTS_H