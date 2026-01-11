#include <iostream>

#include "GameObject.h"
#include "raylib.h"
#include "timer.h"
#include "Universe.h"

class Player : public GameObject {
    static constexpr int DIM = 50;

    Vec2 slashDir = {0, 0};

    GameTimer slashTimer = GameTimer::ofGameTime();

    void update() override {
        if (slashTimer.hasElapsed(0.1)) slashTimer.stop();


        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !slashTimer.isRunning()) {
            const auto mpos = Universe::getMouseWorldPosition();
            const auto pos = getCenterPos();

            slashDir = (mpos - pos).normalizeOrZero();

            slashTimer.reset();
        }

        const Vec2 movDelta = Universe::getVectorInput(KEY_A, KEY_D, KEY_W, KEY_S);\

        position += movDelta * (Universe::getScaledDeltaTime() * 1000);
    }

    [[nodiscard]] Vec2 getCenterPos() const {
        return position + Vec2{DIM / 2.0f, DIM / 2.0f};
    }

    void render2d() override {
        DrawRectangle(static_cast<int>(position.x), static_cast<int>(position.y), DIM, DIM, RED);

        if (slashTimer.isRunning()) {
            const auto desiredPos = getCenterPos() + slashDir * 150;
            DrawCircle(desiredPos.xInt(), desiredPos.yInt(), 50, BLUE);
        }
    }
};

int main() {
    Universe::init(640, 360, "Game",  [] {
        Universe::instantiate(new Player());
    });

    return 0;
}