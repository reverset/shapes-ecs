#ifndef GAME_GAMEOBJECT_H
#define GAME_GAMEOBJECT_H
#include "vec.h"

class GameObject {
public:
    unsigned int processLayer = 0; // TODO

    Vec2 position = VEC2_ZERO;

    virtual ~GameObject() = default;

    virtual void update() {}
    virtual void render2d() {}
};


#endif //GAME_GAMEOBJECT_H