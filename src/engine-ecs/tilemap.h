#ifndef GAME_TILEMAP_H
#define GAME_TILEMAP_H

#include "vector"
#include "raylib.h"

#include "../ecs.h"
#include "../engine-ecs/rendering.h"
#include "../vec.h"

struct SpatialHash {
    std::size_t operator()(const Vec2ui& vec) const noexcept {
        static constexpr std::uint32_t p1 = 73856093;
        static constexpr std::uint32_t p2 = 19349663;
        return (vec.x * p1) ^ (vec.y * p2);
    }
};

struct Tile {
    Sprite sprite;
    Vec2ui position{0, 0};
};

class Tilemap : public Component<Tilemap> {

    // todo layers, and collision
    using TileHashMap = std::unordered_map<Vec2ui, std::vector<Tile>, SpatialHash>;

    TileHashMap tiles;
    std::uint32_t width;
    std::uint32_t height;


    float scalingFactor;
    float inverseScalingFactor;
public:
    COMPONENT_STORAGE(Tilemap);

    std::uint32_t xHashFactor = 16;
    std::uint32_t yHashFactor = 16;

    explicit Tilemap(const std::uint32_t width, const std::uint32_t height, const float scalingFactor) {
        this->width = width;
        this->height = height;
        this->scalingFactor = scalingFactor;
        this->inverseScalingFactor = 1.0f / scalingFactor;
    }

    [[nodiscard]] const TileHashMap& getInternalMap() const {
        return tiles;
    }

    template <typename Func>
    void forEachTile(Func&& f) const {
        for (const auto &val: this->tiles | std::views::values) {
            for (const auto& tile : val) {
                f(tile);
            }
        }
    }

    [[nodiscard]] Vec2 toWorldPosition(const Vec2 tileMapPos, const Vec2ui tileRelativePos) const {
        return tileMapPos + Vec2{
                   static_cast<float>(tileRelativePos.x) * scalingFactor,
                   static_cast<float>(tileRelativePos.y) * scalingFactor
               };
    }

    [[nodiscard]] Vec2ui toRelativePosition(const Vec2 tileMapPos, const Vec2 worldPos) const {
        auto rel = (worldPos - tileMapPos) * inverseScalingFactor;
        rel.x = std::max(rel.x, 0.0f);
        rel.y = std::max(rel.y, 0.0f);

        return {static_cast<std::uint32_t>(rel.x), static_cast<std::uint32_t>(rel.y)};
    }

    [[nodiscard]] bool isPointInBounds(const Vec2ui pos) const {
        return pos.x < width && pos.y < height;
    }

    void insertTile(const Tile& tile) {
        const auto ix = tile.position.x / xHashFactor;
        const auto iy = tile.position.y / yHashFactor;

        const Vec2ui vec{ix, iy};
        tiles[vec].push_back(tile);
    }
};


namespace TilemapSystems {
    inline void renderTilemaps(const Entity, const Tilemap& map, const Transform2d& trans) {
        map.forEachTile([&](const Tile& tile) {
            const auto pos = map.toWorldPosition(trans.position, tile.position);
            tile.sprite.texture->renderEx(pos, tile.sprite.offset, 0.0f, 1.0f, tile.sprite.tint);
        });
    }


    inline void registerAll() {
        Universe::onEarlyRender2d.registerSystem<Tilemap, Transform2d>(renderTilemaps);
    }
}

#endif //GAME_TILEMAP_H