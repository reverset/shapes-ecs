#ifndef GAME_RENDERING_H
#define GAME_RENDERING_H

#include "ecs.h"
#include "Universe.h"
#include "standardcomponents.h"

namespace RenderingSystems {
    inline void renderSprites(const Entity, const Sprite& sprite, const Transform2d& trans) {
        sprite.texture->renderEx(trans.position, sprite.offset, trans.rotation, trans.scale, sprite.tint);
    }

    inline void registerAll() {
        Universe::onRender2d.registerSystem<Sprite, Transform2d>(renderSprites);
    }
}

#endif //GAME_RENDERING_H