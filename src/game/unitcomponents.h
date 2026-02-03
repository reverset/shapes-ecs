#ifndef GAME_UNITCOMPONENTS_H
#define GAME_UNITCOMPONENTS_H

#include "raylib.h"

#include "../engine/Universe.h"
#include "../engine/ecs.h"
#include "../engine/standardcomponents.h"
#include "../engine/logging.h"
#include "../engine/util.h"

struct Health : Component<Health> {
    COMPONENT_STORAGE(Health);

    std::int32_t maxHealth;
    std::int32_t health;

    Timestamp lastDamage = Timestamp::longAgo();

    explicit Health(const std::int32_t maxHealth) {
        this->maxHealth = maxHealth;
        this->health = maxHealth;
    }

    void damage(const std::int32_t dmg) { // todo damage struct
        health = GameUtil::clamp(health - dmg, 0, maxHealth);
        lastDamage = Timestamp::now();
    }

    [[nodiscard]] float getHealthNormalized() const {
        return static_cast<float>(health) / static_cast<float>(maxHealth);
    }
};

struct HealthBar : Component<HealthBar> {
    COMPONENT_STORAGE(HealthBar);

    float width = 30.0f;
    float height = 5.0f;
    float yOffset = -20.0f;

    float damageBarProgress = 1.0f;
};

namespace UnitComponents {
    inline void renderHealthBars(const Entity, HealthBar& bar, const Health& health, const Transform2d& trans) {
        constexpr auto damageCatchupWithHealthDelay = Duration::ofSeconds(0.5);
        constexpr float damageCatchupSpeed = 1.2f;
        
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

    inline void registerAll() {
        Universe::onLateRender2d.registerSystem<HealthBar, Health, Transform2d>(renderHealthBars);
    }
}

#endif //GAME_UNITCOMPONENTS_H