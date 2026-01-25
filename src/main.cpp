#include <iostream>

#include "Files.h"
#include <cstdint>
#include <utility>

#include "GameObject.h"
#include "raylib.h"
#include "timer.h"
#include "Universe.h"
#include "resource.h"
#include "ParticleSystem.h"
#include "ecs.h"

#include "components/standardcomponents.h"
#include "engine-ecs/rendering.h"
#include "engine-ecs/useful.h"

struct Player : Component<Player> {
    COMPONENT_STORAGE(Player);
};

struct WeaponHud : Component<WeaponHud> {
    COMPONENT_STORAGE(WeaponHud);
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

void renderWeaponHud(const Entity, const WeaponHud&, const Transform2d& trans) {
    DrawRectangleRounded(Rectangle{
                         static_cast<float>(Universe::getResolutionX()) - trans.position.x,
                         static_cast<float>(Universe::getResolutionY()) - trans.position.y, 50 * trans.scale, 50 * trans.scale
                     }, 0.5f, 8, RED);
}

void playerMovement(const Entity, const Player&, Transform2d& trans) {
    const Vec2 movDelta = Universe::getInputManager()->testVec2Bind(KeyboardAndMouse, "movement");

    static constexpr int SPEED = 100;
    trans.position += movDelta * (Universe::getScaledDeltaTime() * SPEED);
}

int main() {
    // TODO update gamepad mapping for linux

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
        UsefulSystems::registerAll();

        Universe::onUpdate.registerSystem<Player, Transform2d>(playerMovement);

        Universe::onRenderUi.registerSystem<WeaponHud, Transform2d>(renderWeaponHud);

        const auto playerTexture = *man->getResource<TextureResource>("player");

        auto& es = Universe::getEntityStorage();

        es.makeEntity()
            .addComponent(Player())
            .addComponent(Sprite(playerTexture))
            .addComponent(Transform2d());

        es.makeEntity()
            .addComponent(WeaponHud())
            .addComponent(Transform2d({100, 100}));

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