#ifndef GAME_UIOBJECT_H
#define GAME_UIOBJECT_H

#include <string>

class UIObject {

    virtual void doDraw() {}

public:
    bool visible = true;
    unsigned int layer = 0;
    std::string name;

    void draw() {
        if (visible) doDraw();
    }

    virtual ~UIObject() = default;
};

#endif //GAME_UIOBJECT_H