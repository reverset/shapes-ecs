#include <iostream>

#include "Files.h"
#include <cstdint>
#include <utility>

#include "GameObject.h"
#include "raylib.h"
#include "timer.h"
#include "Universe.h"
#include "resource.h"
#include "UIObject.h"
#include "ParticleSystem.h"
#include "ecs.h"
#include "components/standardcomponents.h"
#include "engine-ecs/rendering.h"

class CameraControllerScript : public GameObject {
    Camera2D* camera = nullptr;
public:
    float zoom = 1.0f;

    CameraControllerScript() {
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

class WeaponHUDScript : public UIObject {
    void doDraw() override {
        DrawRectangleRounded(Rectangle{
                                 static_cast<float>(Universe::getResolutionX()) - 100.0f,
                                 static_cast<float>(Universe::getResolutionY()) - 100.0f, 50, 50
                             }, 0.5f, 8, RED);
    }
};

class PlayerScript : public GameObject {
    static constexpr int DIM = 50;
    static constexpr int SPEED = 100;

    Vec2 slashDir = {0, 0};

    GameTimer slashTimer = GameTimer::ofGameTime();

    TextureResource* tex = nullptr;
    TextureResource* slashEffectTexture = nullptr;

    CPUParticleEmitter* slashEffect = nullptr;

    CameraControllerScript* cameraController = nullptr;


public:
    PlayerScript() {
        name = "Player";
    }

    void ready() override {
        tex = *Universe::getResourceManager()->getResource<TextureResource>("player");
        slashEffectTexture = *Universe::getResourceManager()->getResource<TextureResource>("spark");

        slashEffect = Universe::instantiate(ParticlePresets::pop(5, 0.5, slashEffectTexture, 1.0f));
        slashEffect->setEnabled(false);

        cameraController = Universe::findByName<CameraControllerScript>("CameraController");
        cameraController->zoom = 3.0f;
    }

    void update() override {
        if (slashTimer.hasElapsed(0.1)) {
            slashTimer.stop();
            slashEffect->setEnabled(false); // improve
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !slashTimer.isRunning()) {
            const auto mpos = Universe::getMouseWorldPosition();

            slashDir = (mpos - position).normalizeOrZero();
            const auto desiredPos = position + slashDir * 50;

            slashEffect->position = desiredPos;
            slashEffect->setEnabled(true);

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

struct Player : Component<Player> {
    COMPONENT_STORAGE(Player);
};

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

void playerMovement(const Entity e, const Player&, Transform2d& trans) {
    const Vec2 movDelta = Universe::getInputManager()->testVec2Bind(KeyboardAndMouse, "movement");

    static constexpr int SPEED = 100;
    trans.position += movDelta * (Universe::getScaledDeltaTime() * SPEED);

    if (IsKeyPressed(KEY_J)) {
        Universe::defer([e] {
            Universe::getEntityStorage().destroyEntity(e);
        });
    }
}

int main() {
    Universe::init(640, 360, "Game", [] {
        const auto man = Universe::getResourceManager();

        man->registerResource(
            "floorTile",
            new TextureResource("floorTile.png"));

        man->registerResource(
            "player",
            new TextureResource("player.png"));

        man->registerResource(
            "spark",
            new TextureResource("spark.png"));

        defineKeybindings();

        Universe::getCamera()->zoom = 3.0f;

        RenderingSystems::registerAll();

        Universe::onUpdate.registerSystem<Player, Transform2d>(playerMovement);

        const auto playerTexture = *man->getResource<TextureResource>("player");

        Universe::getEntityStorage().makeEntity()
            .addComponent(Player())
            .addComponent(Sprite(playerTexture))
            .addComponent(Transform2d());
    }, [] {});

    // Universe::init(640, 360, "Game", [] {
    //     const auto man = Universe::getResourceManager();
    //
    //     man->registerResource(
    //         "floorTile",
    //         new TextureResource("floorTile.png"));
    //
    //     man->registerResource(
    //         "player",
    //         new TextureResource("player.png"));
    //
    //     man->registerResource(
    //         "spark",
    //         new TextureResource("spark.png"));
    //
    //     defineKeybindings();
    //
    //     Universe::instantiate(new CameraControllerScript());
    //     Universe::instantiate(new PlayerScript());
    //
    //     Universe::addUiElement(std::move(std::make_unique<WeaponHUDScript>()));
    // }, [] {});

    return 0;
}