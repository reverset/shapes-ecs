#ifndef GAME_PARTICLESYSTEM_H
#define GAME_PARTICLESYSTEM_H

#include <functional>

#include "../engine/vec.h"
#include "../engine/timer.h"
#include "../engine/standardcomponents.h"

namespace Particles {
    inline void sparkle(const Vec2 pos, const std::size_t particles = 4, const Color color = WHITE) {
        if (Universe::areEntitiesBusy()) {
            Universe::defer([=] {sparkle(pos, particles, color);});
            return;
        }

        const auto sparkleTexture = AssetStore::getSparkTexture();

        for (std::size_t i = 0; i < particles; i++) {
            constexpr auto lifetime = Duration::ofSeconds(1.0);

            Universe::getEntityStorage()
                .makeEntity()
                    .addComponent(Transform2d(pos + Vec2::randomDirection(3)))
                    .addComponent(Velocity(RandomGen::randomFloat(-50, 50), RandomGen::randomFloat(-50, -20), RandomGen::randomFloat(-3, 3)))
                    .addComponent(ConstantForce(0, 100))
                    .addComponent(Sprite(sparkleTexture, color))
                    .addComponent(FadeOverTime(lifetime))
                    .addComponent(RenderLayer5())
                    .addComponent(Transient(lifetime));
        }
    }
}

#endif //GAME_PARTICLESYSTEM_H