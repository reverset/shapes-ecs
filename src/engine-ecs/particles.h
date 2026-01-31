#ifndef GAME_PARTICLESYSTEM_H
#define GAME_PARTICLESYSTEM_H

#include <vector>
#include <functional>
#include <stdexcept>

#include "../vec.h"
#include "../util.h"
#include "../timer.h"
#include "useful.h"
#include "standardcomponents.h"
#include "rendering.h"

namespace Particles {
    void sparkle(Vec2 pos) {
        if (Universe::areEntitiesBusy()) {
            Universe::defer([=] {sparkle(pos);});
            return;
        }

        const auto sparkleTexture = *Universe::getResourceManager()->getResource<TextureResource>("spark");
        
        const std::size_t particles = 4;
        for (std::size_t i = 0; i < particles; i++) {
            const auto lifetime = Duration::ofSeconds(1.0);
            Universe::getEntityStorage()
                .makeEntity()
                    .addComponent(Transform2d(pos + Vec2::randomDirection(3)))
                    .addComponent(Velocity(RandomGen::random(-50, 50), RandomGen::random(-50, -20), RandomGen::randomFloat(-3, 3)))
                    .addComponent(ConstantForce(0, 100))
                    .addComponent(Sprite(sparkleTexture))
                    .addComponent(FadeOverTime(lifetime))
                    .addComponent(Transient(lifetime));
        }
    }
}

// TODO: variable lifetime (random), self-destruction of particle emitter. (perhaps change architecture to an ECS?)

// class CPUParticleEmitter;
//
// struct CPUParticleHandle {
//     size_t id;
//     CPUParticleEmitter* emitter;
//
//     [[nodiscard]] Vec2& position() const;
//
//     [[nodiscard]] Vec2 &velocity() const;
//
//     [[nodiscard]] double normalLifetime() const;
// };
//
// typedef std::function<void(CPUParticleHandle)> ParticleInitalizer;
//
// typedef std::function<Color(CPUParticleHandle)> ColorSupplier;
// typedef std::function<void(CPUParticleHandle)> PositionSupplier;
// typedef std::function<void(CPUParticleHandle)> VelocitySupplier;
// typedef std::function<void(CPUParticleHandle, Color)> ParticleRenderer;
//
// // TODO, revise for ECS
// class CPUParticleEmitter {
//     std::vector<Vec2> particlePositions;
//     std::vector<Vec2> particleVelocities;
//     std::vector<double> particleLifetime;
//
//     static void standardParticlePositionIntegrator(const CPUParticleHandle part) {
//         part.position() += part.velocity() * Universe::getScaledDeltaTime();
//     }
//
//     static Color standardParticleTint(const CPUParticleHandle) {
//         return WHITE;
//     }
//
//     static void standardParticleRenderer(const CPUParticleHandle part, const Color col) {
//         DrawCircle(static_cast<int>(part.position().x + part.emitter->position.x),
//                    static_cast<int>(part.position().y + part.emitter->position.y), 3.0f, col);
//     }
//
//     ParticleInitalizer particleInit = nullptr;
//
//     ColorSupplier colorSup = standardParticleTint;
//     PositionSupplier positionSup = standardParticlePositionIntegrator;
//     VelocitySupplier velocitySup = nullptr;
//     ParticleRenderer renderer = standardParticleRenderer;
//
//     double spawnEvery = 0;
//
//     unsigned int maxParticles;
//     double maxLifetime;
//     unsigned int particlesPerSpawn;
//     bool isEmitting;
//
//     GameTimer spawnTimer = GameTimer::ofGameTime();
//
//     void makeParticle() {
//         const auto part = CPUParticleHandle {
//             .id = size(),
//             .emitter = this,
//         };
//         particlePositions.push_back(VEC2_ZERO);
//         particleVelocities.push_back(VEC2_ZERO);
//         particleLifetime.push_back(0);
//
//         if (particleInit != nullptr) {
//             particleInit(part);
//         }
//     }
//
//     void eraseParticleAt(const size_t id) {
//         particlePositions.erase(particlePositions.begin() + id);
//         particleVelocities.erase(particleVelocities.begin() + id);
//         particleLifetime.erase(particleLifetime.begin() + id);
//     }
//
// public:
//     Vec2 position = {0, 0};
//
//     explicit CPUParticleEmitter(const bool spawnEnabled, const double maxLifetime, const unsigned int maxParticles, const double spawnEvery, const unsigned int particlesPerSpawn) {
//         this->isEmitting = spawnEnabled;
//         this->maxLifetime = maxLifetime;
//         this->maxParticles = maxParticles;
//         this->spawnEvery = spawnEvery;
//         this->particlesPerSpawn = particlesPerSpawn;
//     }
//
//     [[nodiscard]] size_t size() const {
//         return particlePositions.size();
//     }
//
//     CPUParticleEmitter* withParticleInitializer(const ParticleInitalizer& partInit) {
//         particleInit = partInit;
//         return this;
//     }
//
//
//     CPUParticleEmitter* withColorSupplier(const ColorSupplier& colSup) {
//         colorSup = colSup;
//         return this;
//     }
//
//     CPUParticleEmitter* withPositionSupplier(const PositionSupplier& posSup) {
//         positionSup = posSup;
//         return this;
//     }
//
//     CPUParticleEmitter* withVelocitySupplier(const VelocitySupplier& velSup) {
//         velocitySup = velSup;
//         return this;
//     }
//
//     CPUParticleEmitter* withRenderer(const ParticleRenderer& partRenderer) {
//         renderer = partRenderer;
//         return this;
//     }
//
//     void setEnabled(const bool enabled) {
//         isEmitting = enabled;
//         if (isEmitting) spawnTimer.reset();
//     }
//
//     void ready() {
//         spawnTimer.reset();
//     }
//
//     void update() {
//         if (isEmitting && size() < maxParticles && spawnTimer.hasElapsedAdvance(spawnEvery)) {
//             for (int i = 0; i < particlesPerSpawn && size() < maxParticles; i++) {
//                 makeParticle();
//             }
//         }
//     }
//
//     void render2d() {
//         if (colorSup == nullptr) throw std::runtime_error("No color supplied");
//         if (positionSup == nullptr) throw std::runtime_error("No position supplied");
//         if (renderer == nullptr) throw std::runtime_error("No renderer supplied");
//
//         for (size_t i = 0; i < size(); i++) {
//             particleLifetime[i] += Universe::getScaledDeltaTime();
//             if (particleLifetime[i] > maxLifetime) { // erase expired particles
//                 eraseParticleAt(i);
//                 i -= 1;
//                 continue;
//             }
//
//             const auto part = CPUParticleHandle{
//                 .id = i,
//                 .emitter = this,
//             };
//
//
//             positionSup(part);
//             if (velocitySup != nullptr) velocitySup(part);
//             renderer(part, colorSup(part));
//         }
//     }
//
//     friend CPUParticleHandle;
// };
//
// inline Vec2& CPUParticleHandle::position() const {
//     return emitter->particlePositions[id];
// }
//
// inline Vec2& CPUParticleHandle::velocity() const {
//     return emitter->particleVelocities[id];
// }
//
// inline double CPUParticleHandle::normalLifetime() const {
//     return GameUtil::clamp(emitter->particleLifetime[id] / emitter->maxLifetime, 0.0, 1.0);
// }
//
// namespace ParticlePresets {
//     inline ParticleInitalizer randomVelocity(const float speed) {
//         return [speed](const CPUParticleHandle part) {
//             part.velocity() = Vec2::randomDirection(speed);
//         };
//     }
//
//     inline VelocitySupplier slowDownOverLifetime(const float coeff) {
//         return [coeff](const CPUParticleHandle part) {
//             part.velocity() = part.velocity().moveTowardsMagnitude(0, Universe::getScaledDeltaTime() * coeff);
//         };
//     }
//
//     inline ParticleRenderer texture(TextureResource* tex, const float scale) {
//         return [tex, scale](const CPUParticleHandle part, const Color tint) {
//             tex->render(part.position() + part.emitter->position, 0.0f, scale, tint);
//         };
//     }
//
//     inline CPUParticleEmitter* pop(const unsigned int maxParticles, const double lifetime, TextureResource* tex, const float scale) {
//         auto* emitter = new CPUParticleEmitter(true, lifetime, maxParticles, 0.0, maxParticles);
//         emitter
//             ->withParticleInitializer(randomVelocity(50.0))
//             ->withVelocitySupplier(slowDownOverLifetime(80.0))
//             ->withRenderer(texture(tex, scale));
//
//         return emitter;
//     }
// }

#endif //GAME_PARTICLESYSTEM_H