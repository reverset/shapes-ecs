#include <fstream>
#include <iostream>


#include "assetstore.h"
#include "BitLayers.h"
#include "enemies.h"
#include "raylib.h"
#include "../engine/Universe.h"
#include "../engine/resource.h"
#include "../engine/ecs.h"
#include "../engine/timer.h"
#include "../engine/Files.h"
#include "particles.h"
#include "unitcomponents.h"

#include "../engine/standardcomponents.h"
#include "../engine/tilemap.h"
#include "../engine/ui.h"
#include "../engine/anim.h"
#include "GLFW/glfw3.h"

struct PlayerHealthBar : Component<PlayerHealthBar> {
    COMPONENT_STORAGE(PlayerHealthBar);
};

void defineKeybindings() {
    const auto input = Universe::getInputManager();
    // TODO, deadzone
    input->bindVector2("movement", Vec2GamepadBinding {
        .keyboard = [] { return Universe::getVectorInput(KEY_A, KEY_D, KEY_W, KEY_S); },
        .gamepad = [](const int id) { return Vec2{GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_X), GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_Y)}; },
    });

    input->bindBoolean("shoot", BooleanGamepadBinding {
        .keyboard = [] { return IsMouseButtonDown(MOUSE_BUTTON_LEFT); },
        .gamepad = [](const int id) { return GetGamepadAxisMovement(id, GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.5f; },
    });
}

void playerMovement(const Entity, const Player&, Transform2d& trans) {
    const Vec2 movDelta = Universe::getInputManager()->testVec2Bind(KeyboardAndMouse, "movement");

    static constexpr int SPEED = 100;
    trans.position += movDelta * (Universe::getScaledDeltaTime() * SPEED);
}

void playerAttackControl(const Entity e, Player& player, const Transform2d& trans) {
    constexpr double BULLET_SPEED = 200.0;

    auto& weapon = player.heldWeapon;

    if (weapon.lastFiredTimestamp.hasElapsed(weapon.cooldownTime)
        && Universe::getInputManager()->testBooleanBind(KeyboardAndMouse, "shoot")) {

        weapon.lastFiredTimestamp = Timestamp::now();

        const auto mouse = Universe::getMouseWorldPosition();
        const auto direction = (mouse - trans.position).normalizeOrZero();
        const auto vel = direction * BULLET_SPEED;

        Universe::defer([=] {
            const auto bullet = Spawning::spawnBullet(e, 10, trans.position, vel, 1.0f, BitLayers::ENEMY_LAYER);
            Universe::getEntityStorage().insertComponent(bullet, TilemapCollider(8, 8));
        });
    }
}

// TODO, use velocity to move camera 'ahead' of player for better vision.
void centerCameraOnPlayer(const Entity, const Player&, const Transform2d& trans) {
    const auto cam = Universe::getCamera();
    // cam->target = trans.position;
    cam->target = static_cast<Vec2>(cam->target).lerp(trans.position, Universe::getScaledDeltaTime() * 2.0f);
}

void rightClickPlaceTile(const Entity, Tilemap& map, const Transform2d& trans) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        const auto desired = map.toRelativePosition(trans.position, Universe::getMouseWorldPosition());

        if (desired.has_value()) {
            map.insertTile(0, Tile {
                .sprite = Sprite(*Universe::getResourceManager()->getResource<TextureResource>("grassTile")),
                .position = desired.value(),
            });
        }
    }
}

void handleBulletHitTile(const TilemapCollisionEvent& e) {
    ECS::queryComponentsFor<Bullet, Transform2d>(e.collided, [](const Entity bullet, const Bullet&, const Transform2d& trans) {
        Particles::sparkle(trans.position);
        Universe::getEntityStorage().destroyEntity(bullet);
    });
}

void handleBulletHitWithVolume(const OnDamageDealtByVolume& e) {
    ECS::queryComponentsFor<Bullet, Transform2d>(e.attacker, [](const Entity bullet, const Bullet&, const Transform2d& trans) {
        Particles::sparkle(trans.position);
        Universe::getEntityStorage().destroyEntity(bullet);
    });
}

void renderPlayerHealthBar(const Entity, const PlayerHealthBar&, const Health& hp) {
    const auto pos = UI::percentFromBLCorner({0.05, 0.1});

    constexpr float maxLength = 200.0f;
    constexpr float barHeight = 25.0f;

    const auto desiredLength = hp.getHealthNormalized() * maxLength;

    DrawRectangleRounded({pos.x, pos.y, maxLength, barHeight}, 0.8, 4, DARKERGRAY);
    DrawRectangleRounded({pos.x, pos.y, desiredLength, barHeight}, 0.8, 4, RED);
}

void loadMap() {
    // auto& es = Universe::getEntityStorage();

    // const auto tileTexture = AssetStore::getGrassTexture();
    // const auto genericWallTexture = AssetStore::getGenericWallTexture();

    // auto map = Tilemap(100, 100, 16.0f, 3);
    // // map.insertTile(0, Tile {
    // //     .sprite = Sprite(grassTexture),
    // //     .position = {0, 0},
    // // });
    //
    // map.fillTile(0, Sprite(tileTexture), {0, 0}, {100, 100});
    //
    // map.insertTile(1, Tile{
    //     .sprite = Sprite(genericWallTexture),
    //     .position = Vec2ui{5, 5},
    // }, true);
    //
    // map.insertTile(1, Tile{
    //     .sprite = Sprite(genericWallTexture),
    //     .position = Vec2ui{7, 5},
    // }, true);
    //
    // map.fillTile(1, Sprite(genericWallTexture), {8, 8}, {12, 12}, true);
    //
    // map.cacheAll();

    // es.makeEntity()
    //     .addComponent(std::move(map))
    //     .addComponent(Transform2d());

    // const auto testPath = map.calculatePath(Vec2ui::zero(), {15, 15}, 0);
    // Logging::log("testPath size=%d", testPath.size());
    //
    // for (const auto relPos : testPath) {
    //     const auto pos = map.toWorldPosition({0, 0}, relPos);
    //
    //     es.makeEntity()
    //         .addComponent(DebugMarker{})
    //         .addComponent(Transform2d(pos));
    // }
}

#ifndef NDEBUG
void debugButtons(const Entity, const Player&, const Transform2d& trans) {
    // auto& es = Universe::getEntityStorage();
    if (IsKeyPressed(KEY_SLASH)) {
        Logging::log("spawning meanie");
        const auto desiredPos = trans.position + Vec2::randomDirection(200);
        Enemies::spawnMeanie(desiredPos);
    } else if (IsKeyPressed(KEY_PERIOD)) {
        Logging::log("spawning targeter");
        const auto desiredPos = trans.position + Vec2::randomDirection(50);
        Enemies::spawnTargeter(desiredPos, 2);
    } else if (IsKeyPressed(KEY_COMMA)) {
        Logging::log("spawning heart");
        const auto desiredPos = trans.position + Vec2::randomDirection(30);
        Spawning::spawnHealingHeart(10, desiredPos, Vec2::zero());
    } else if (IsKeyPressed(KEY_M)) {
        // SetConfigFlags(FLAG_VSYNC_HINT);
        glfwSwapInterval(1);
    }
}
#endif

int main() {
    // TODO update gamepad mapping for linux ... MIGHT BE MORE PROBLEMATIC THAN ANTICIPATED
    
    Universe::init(640, 360, "Pixel Space", [] {
        AssetStore::initialLoadAll();

        defineKeybindings();

        Universe::getCamera()->zoom = 3.0f;
        // Universe::getCamera()->zoom = 0.5f;
 
        RenderingSystems::registerAll();
        StandardComponentSystems::registerAll();
        TilemapSystems::registerAll();
        Enemies::registerAll();
        UnitComponents::registerAll();
        // StandardComponentSystems::enableDebugRendering();

        Universe::onUpdate
            .registerSystem<Player, Transform2d>(playerMovement)
            .registerSystem<Player, Transform2d>(playerAttackControl)
            .registerSystem<Velocity, Transform2d>(StandardComponentSystems::applyVelocity)
            .registerSystem<Tilemap, Transform2d>(rightClickPlaceTile);

        Universe::onLateUpdate
            .registerSystem<Player, Transform2d>(centerCameraOnPlayer);

        Universe::onRenderUi
            .registerSystem<PlayerHealthBar, Health>(renderPlayerHealthBar);

        #ifndef NDEBUG
        Universe::onFinalFrameUpdate
            .registerSystem<Player, Transform2d>(debugButtons);
        #endif

        // Universe::onEarlyRender2d
        //     .registerSystem<Tilemap, Transform2d>(TilemapSystems::renderTilemapsChunked);

        TilemapCollisionEvent::listen(handleBulletHitTile);
        OnDamageDealtByVolume::listen(handleBulletHitWithVolume);

        const auto playerTexture = AssetStore::getPlayerTexture();

        auto& es = Universe::getEntityStorage();

        loadMap();

        es.makeEntity()
            .addComponent(Player())
            .addComponent(Sprite(playerTexture))
            .addComponent(Transform2d())
            // .addComponent(TilemapRenderTracker())
            // .addComponent(TilemapCollider(16, 16))
            .addComponent(RenderLayer3())
            .addComponent(HealthInteractionVolume(BitLayers::PLAYER_LAYER, {16, 16}))
            .addComponent(CollisionRect(16, 16, BitLayers::PLAYER_LAYER, BitLayers::NONE))
            .addComponent(Health(200))
            .addComponent(PlayerHealthBar());

        StandardEntityPresets::makeBackground(AssetStore::getBackgroundTexture(), 0.08f);

    }, [] {});
    // }, [] {});

    return 0;
}
