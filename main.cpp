#include <iostream>

#include "Files.h"
#include <cstdint>

#include "GameObject.h"
#include "raylib.h"
#include "timer.h"
#include "Universe.h"
#include "resource.h"
#include "UIObject.h"

class CameraController : public GameObject {
    Camera2D* camera = nullptr;
public:
    float zoom = 1.0f;

    CameraController() {
        name = "CameraController";
        processLayer = UINT32_MAX;
    }

    void ready() override {
        camera = Universe::getCamera();
    }

    void update() override { // maybe replace with just setters & getters
        camera->target = position;
        camera->zoom = zoom;
    }

};

class WeaponHUD : public UIObject {
    void doDraw() override {
        DrawRectangleRounded(Rectangle{Universe::getResolutionX() - 100.0f, Universe::getResolutionY() - 100.0f, 50, 50}, 0.5f, 8, RED);
    }
};

class Player : public GameObject {
    static constexpr int DIM = 50;
    static constexpr int SPEED = 100;

    Vec2 slashDir = {0, 0};

    GameTimer slashTimer = GameTimer::ofGameTime();
    TextureResource* tex = nullptr;
    CameraController* cameraController = nullptr;

public:
    Player() {
        name = "Player";
    }

    void ready() override {
        tex = *Universe::getResourceManager()->getResource<TextureResource>("player");
        cameraController = Universe::findByName<CameraController>("CameraController");
        cameraController->zoom = 3.0f;
    }

    void update() override {
        if (slashTimer.hasElapsed(0.1)) slashTimer.stop();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !slashTimer.isRunning()) {
            const auto mpos = Universe::getMouseWorldPosition();

            slashDir = (mpos - position).normalizeOrZero();

            slashTimer.reset();
        }

        const Vec2 movDelta = Universe::getVectorInput(KEY_A, KEY_D, KEY_W, KEY_S);

        position += movDelta * (Universe::getScaledDeltaTime() * SPEED);
    }

    void render2d() override {
        tex->render(position, 0.0, 1.0f, WHITE);

        if (slashTimer.isRunning()) {
            const auto desiredPos = position + slashDir * 50;
            DrawCircle(desiredPos.xInt(), desiredPos.yInt(), 10, BLUE);
        }
    }
};

int main() {
    Universe::init(640, 360, "Game", [] {
        const auto man = Universe::getResourceManager();

        man->registerResource(
            "floorTile",
            new TextureResource("floorTile.png"));

        man->registerResource(
            "player",
            new TextureResource("player.png"));

        Universe::instantiate(new CameraController());
        Universe::instantiate(new Player());

        Universe::addUiElement(std::make_unique<WeaponHUD>());
    }, [] {
    });

    return 0;
}