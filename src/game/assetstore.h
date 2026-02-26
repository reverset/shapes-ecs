#ifndef GAME_ASSETSTORE_H
#define GAME_ASSETSTORE_H

#include "../engine/resource.h"
#include "../engine/Universe.h"

namespace AssetStore {

    TextureResource* getHeartTexture();
    inline void loadHealingHeart() {
        const auto man = Universe::getResourceManager();

        const auto heart = getHeartTexture();

        man->registerResource(
            "healing-heart",
            TextureResource::generate([&] {
                const auto rtex = LoadRenderTexture(32, 32);
                
                BeginTextureMode(rtex);
                ClearBackground(BLANK);
                
                heart->render({16, 16}, PI, 1.0f, PINK);

                EndTextureMode();

                return rtex;
            }
        ));
    }

    inline void initialLoadAll() {
        const auto man = Universe::getResourceManager();
        man->registerResource(
            "genericWall",
            new TextureResource("genericWall.png"));

        man->registerResource(
            "player",
            new TextureResource("player.png"));

        man->registerResource(
            "spark",
            new TextureResource("spark.png"));

        man->registerResource(
            "bullet",
            new TextureResource("bullet.png"));

        man->registerResource(
            "floor",
            new TextureResource("floor.png"));

        man->registerResource(
            "meanie",
            new TextureResource("meanie.png"));

        man->registerResource(
            "target",
            new TextureResource("target.png"));

        man->registerResource(
            "shield-bubble",
            new TextureResource("shield-bubble.png"));

        man->registerResource(
            "pulse-effect",
            TextureResource::generate([] {
                const auto rtex = LoadRenderTexture(16, 16);
                BeginTextureMode(rtex);

                DrawCircle(8, 8, 8, WHITE);

                EndTextureMode();

                return rtex;
            }));

        man->registerResource(
            "test-fs",
            new FragmentShader("shaders/test.fs"));

        man->registerResource(
            "outline-fs",
            new FragmentShader("shaders/outline.fs"));

        man->registerResource(
            "healing-heart-fs",
            new FragmentShader("shaders/healing_heart.fs"));

        man->registerResource(
            "heart",
            new TextureResource("heart.png"));

        loadHealingHeart();
    }

    // no more typos
    inline TextureResource* getGenericWallTexture() {
        return *Universe::getResourceManager()->getResource<TextureResource>("genericWall");
    }

    inline TextureResource* getGrassTexture() {
        return *Universe::getResourceManager()->getResource<TextureResource>("floor");
    }

    inline TextureResource* getSparkTexture() {
        return *Universe::getResourceManager()->getResource<TextureResource>("spark");
    }

    inline TextureResource* getBulletTexture() {
        return *Universe::getResourceManager()->getResource<TextureResource>("bullet");
    }

    inline TextureResource* getMeanieTexture() {
        return *Universe::getResourceManager()->getResource<TextureResource>("meanie");
    }

    inline TextureResource* getPlayerTexture() {
        return *Universe::getResourceManager()->getResource<TextureResource>("player");
    }

    inline TextureResource* getTargetTexture() {
        return *Universe::getResourceManager()->getResource<TextureResource>("target");
    }

    inline TextureResource* getShieldBubbleTexture() {
        return *Universe::getResourceManager()->getResource<TextureResource>("shield-bubble");
    }

    inline TextureResource* getPulseEffect() {
        return *Universe::getResourceManager()->getResource<TextureResource>("pulse-effect");
    }

    inline TextureResource* getHeartTexture() {
        return *Universe::getResourceManager()->getResource<TextureResource>("heart");
    }

    inline TextureResource* getHealingHeart() {
        return *Universe::getResourceManager()->getResource<TextureResource>("healing-heart");
    }

    inline FragmentShader* getTestShader() {
        return *Universe::getResourceManager()->getResource<FragmentShader>("test-fs");
    }

    inline FragmentShader* getOutlineShader() {
        return *Universe::getResourceManager()->getResource<FragmentShader>("outline-fs");
    }

    inline FragmentShader* getHealingHeartShader() {
        return *Universe::getResourceManager()->getResource<FragmentShader>("healing-heart-fs");
    }
}

#endif //GAME_ASSETSTORE_H