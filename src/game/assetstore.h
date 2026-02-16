#ifndef GAME_ASSETSTORE_H
#define GAME_ASSETSTORE_H

#include "../engine/resource.h"
#include "../engine/Universe.h"

namespace AssetStore {
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
}

#endif //GAME_ASSETSTORE_H