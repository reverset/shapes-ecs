#include <iostream>

#include "Files.h"
#include <cstdint>

#include "GameObject.h"
#include "raylib.h"
#include "timer.h"
#include "Universe.h"
#include "resource.h"
#include "UIObject.h"
#include "GLFW/glfw3.h"

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

        const Vec2 movDelta = Universe::getInputManager()->testVec2Bind(KeyboardAndMouse, "movement");

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

void defineKeybindings();

int main() {
    Universe::init(640, 360, "Game", [] {
        const auto man = Universe::getResourceManager();

        man->registerResource(
            "floorTile",
            new TextureResource("floorTile.png"));

        man->registerResource(
            "player",
            new TextureResource("player.png"));

        defineKeybindings();

        Universe::instantiate(new CameraController());
        Universe::instantiate(new Player());

        Universe::addUiElement(std::move(std::make_unique<WeaponHUD>()));
    }, [] {
    });

    return 0;
}

void defineKeybindings() {
    // Universe::getInputManager()->bindBoolean("moveUp", BooleanGamepadBinding { // testing
    //     .keyboard = []{ return IsKeyPressed(KEY_W); },
    //     .gamepad = [](const int id){ return IsGamepadButtonPressed(id, GAMEPAD_BUTTON_RIGHT_FACE_DOWN); },
    // });

    // TODO, deadzone
    Universe::getInputManager()->bindVector2("movement", Vec2GamepadBinding {
        .keyboard = [] { return Universe::getVectorInput(KEY_A, KEY_D, KEY_W, KEY_S); },
        .gamepad = [](const int id) { return Vec2{GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_X), GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_Y)}; },
    });
}