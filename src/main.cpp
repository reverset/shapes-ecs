#include <iostream>

#include "Files.h"

#include "raylib.h"
#include "Universe.h"
#include "resource.h"
#include "ParticleSystem.h"
#include "ecs.h"
#include "timer.h"

#include "components/standardcomponents.h"
#include "engine-ecs/rendering.h"
#include "engine-ecs/useful.h"
#include "engine-ecs/tilemap.h"

struct Weapon {
    Timestamp lastFiredTimestamp{};
    Duration cooldownTime{};

    explicit Weapon(const Duration cooldownTime) {
        this->cooldownTime = cooldownTime;
    }
};

struct Player : Component<Player> {
    COMPONENT_STORAGE(Player);

    Weapon heldWeapon{Duration::ofSeconds(0.2)};
};

struct WeaponHud : Component<WeaponHud> {
    COMPONENT_STORAGE(WeaponHud);
};

struct Bullet : Component<Bullet> {
    COMPONENT_STORAGE(Bullet);

    Entity attacker{};
    std::uint32_t baseDamage;

    Bullet() = delete;

    explicit Bullet(const Entity attacker, const std::uint32_t baseDamage) {
        this->attacker = attacker;
        this->baseDamage = baseDamage;
    }
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

void spawnBullet(const Entity attacker, const std::uint32_t baseDamage, const Vec2 pos, const Vec2 vel, float hitboxScale, const std::string& spriteName = "bullet") {
    const auto sprite = *Universe::getResourceManager()->getResource<TextureResource>(spriteName);

    auto& store = Universe::getEntityStorage();
    store.makeEntity()
        .addComponent(Sprite(sprite))
        .addComponent(Transform2d(pos, vel.toAngle(), 0.8f))
        .addComponent(Bullet(attacker, baseDamage))
        .addComponent(Velocity(vel))
        .addComponent(Transient{Duration::ofSeconds(1.0)});
}

void playerAttackControl(const Entity e, Player& player, const Transform2d& trans) {
    constexpr double BULLET_SPEED = 200.0;

    auto& weapon = player.heldWeapon;

    if (weapon.lastFiredTimestamp.hasElasped(weapon.cooldownTime)
        && Universe::getInputManager()->testBooleanBind(KeyboardAndMouse, "shoot")) {

        weapon.lastFiredTimestamp = Timestamp{};

        const auto mouse = Universe::getMouseWorldPosition();
        const auto direction = (mouse - trans.position).normalizeOrZero();
        const auto vel = direction * BULLET_SPEED;

        Universe::defer([=] {
            spawnBullet(e, 10, trans.position, vel, 1.0f);
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
        Logging::log("desired=%s", desired.toString().c_str());

        if (map.isPointInBounds(desired)) {
            map.insertTile(Tile {
                .sprite = Sprite(*Universe::getResourceManager()->getResource<TextureResource>("grassTile")),
                .position = desired,
            });
        }
    }
}

void loadMap() {
    const auto man = Universe::getResourceManager();
    auto& es = Universe::getEntityStorage();

    const auto grassTexture = *man->getResource<TextureResource>("grassTile");

    auto map = Tilemap(10, 10, 16.0f);
    map.insertTile(Tile {
        .sprite = Sprite(grassTexture),
        .position = {0, 0},
    });

    es.makeEntity()
        .addComponent(std::move(map))
        .addComponent(Transform2d());
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

        man->registerResource(
            "bullet",
            new TextureResource("bullet.png"));

        man->registerResource(
            "grassTile",
            new TextureResource("grass.png"));

        defineKeybindings();

        Universe::getCamera()->zoom = 3.0f;

        RenderingSystems::registerAll();
        UsefulSystems::registerAll();
        TilemapSystems::registerAll();

        Universe::onUpdate
            .registerSystem<Player, Transform2d>(playerMovement)
            .registerSystem<Player, Transform2d>(playerAttackControl)
            .registerSystem<Velocity, Transform2d>(UsefulSystems::applyVelocity)
            .registerSystem<Tilemap, Transform2d>(rightClickPlaceTile);

        Universe::onLateUpdate
            .registerSystem<Player, Transform2d>(centerCameraOnPlayer);

        Universe::onRenderUi.registerSystem<WeaponHud, Transform2d>(renderWeaponHud);

        const auto playerTexture = *man->getResource<TextureResource>("player");

        auto& es = Universe::getEntityStorage();

        loadMap();

        es.makeEntity()
            .addComponent(Player())
            .addComponent(Sprite(playerTexture))
            .addComponent(Transform2d());

        es.makeEntity()
            .addComponent(WeaponHud())
            .addComponent(Transform2d({100, 100}));

    }, [] {});
    // }, [] {});

    return 0;
}