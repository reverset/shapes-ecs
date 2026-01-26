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

    std::vector<TileHashMap> tiles;
    std::uint32_t width;
    std::uint32_t height;

    float scalingFactor;
    float inverseScalingFactor;

public:
    COMPONENT_STORAGE(Tilemap);

    std::uint32_t xHashFactor = 16;
    std::uint32_t yHashFactor = 16;

    explicit Tilemap(const std::uint32_t width, const std::uint32_t height, const float scalingFactor, const std::size_t layers) {
        this->width = width;
        this->height = height;
        this->scalingFactor = scalingFactor;
        this->inverseScalingFactor = 1.0f / scalingFactor;

        for (std::size_t i = 0; i < layers; ++i) {
            tiles.emplace_back();
        }
    }

    [[nodiscard]] constexpr std::size_t getLayerCount() const noexcept {
        return tiles.size();
    }

    [[nodiscard]] TileHashMap& getLayer(const std::size_t layer) {
        return tiles.at(layer);
    }

    [[nodiscard]] const TileHashMap& getLayerView(const std::size_t layer) const {
        return tiles.at(layer);
    }

    [[nodiscard]] const std::vector<Tile>& getTileView(const std::size_t layer, const Vec2ui hash) {
        return tiles.at(layer)[hash];
    }

    template <typename Func>
    void forEachTile(Func&& f) const {
        for (std::size_t i = 0; i < tiles.size(); ++i) {
            for (const auto &val: getLayerView(i) | std::views::values) {
                for (const auto& tile : val) {
                    f(tile);
                }
            }
        }
    }

    [[nodiscard]] Vec2 toWorldPosition(const Vec2 tileMapPos, const Vec2ui tileRelativePos) const {
        return tileMapPos + Vec2{
                   static_cast<float>(tileRelativePos.x) * scalingFactor,
                   static_cast<float>(tileRelativePos.y) * scalingFactor
               };
    }

    [[nodiscard]] Vec2ui toRelativePositionClamped(const Vec2 tileMapPos, const Vec2 worldPos) const {
        auto rel = (worldPos - tileMapPos) * inverseScalingFactor;
        rel.x = GameUtil::clamp(rel.x, 0.0f, static_cast<float>(width));
        rel.y = GameUtil::clamp(rel.y, 0.0f, static_cast<float>(height));

        return {static_cast<std::uint32_t>(rel.x), static_cast<std::uint32_t>(rel.y)};
    }

    [[nodiscard]] Vec2ui getClosestHashLocation(const Vec2 tileMapWorldPos, const Vec2 worldPos) const {
        auto rel = (worldPos - tileMapWorldPos) * Vec2(1.0f / static_cast<float>(xHashFactor), 1.0f / static_cast<float>(yHashFactor)) * inverseScalingFactor;
        rel.x = std::max(rel.x, 0.0f);
        rel.y = std::max(rel.y, 0.0f);

        return {static_cast<std::uint32_t>(rel.x), static_cast<std::uint32_t>(rel.y)};
    }

    [[nodiscard]] std::optional<Vec2ui> toRelativePosition(const Vec2 tileMapPos, const Vec2 worldPos) const {
        const auto rel = (worldPos - tileMapPos) * inverseScalingFactor;
        if (rel.x < 0 || rel.y < 0 || rel.x >= static_cast<float>(width) || rel.y >= static_cast<float>(height)) return std::nullopt;

        return Vec2ui{static_cast<std::uint32_t>(rel.x), static_cast<std::uint32_t>(rel.y)};
    }

    [[nodiscard]] bool isPointInBounds(const Vec2ui pos) const {
        return pos.x < width && pos.y < height;
    }

    [[nodiscard]] bool isWorldPointInBounds(const Vec2 tileMapPos, const Vec2 worldPos) const {
        const auto point = toRelativePosition(tileMapPos, worldPos);
        return point.has_value();
    }

    void insertTile(const std::size_t layer, const Tile& tile) {
        auto& map = getLayer(layer);

        const auto ix = tile.position.x / xHashFactor;
        const auto iy = tile.position.y / yHashFactor;

        const Vec2ui vec{ix, iy};
        map[vec].push_back(tile);
    }

    void fillTile(const std::size_t layer, const Sprite& sprite, const Vec2ui start, const Vec2ui end) {
        for (std::uint32_t x = start.x; x < end.x; ++x) {
            for (std::uint32_t y = start.y; y < end.y; ++y) {
                insertTile(layer, Tile {
                    .sprite = sprite,
                    .position = {x, y},
                });
            }
        }
    }
};

struct TilemapRenderTracker : Component<TilemapRenderTracker> {
    COMPONENT_STORAGE(TilemapRenderTracker);
};

namespace TilemapSystems {
    inline void renderTilemapsInFull(const Entity, const Tilemap& map, const Transform2d& trans) {
        map.forEachTile([&](const Tile& tile) {
            const auto pos = map.toWorldPosition(trans.position, tile.position);
            tile.sprite.texture->renderEx(pos, tile.sprite.offset, 0.0f, 1.0f, tile.sprite.tint);
        });
    }

    // TODO: part of this function does not have to run as frequently as the actual rendering part. (hashing part can run wayyy less frequently)
    // consider optimization if necessary
    inline void renderTilemapsChunked(const Entity, Tilemap& map, const Transform2d& trans) {
        const std::optional<std::tuple<TilemapRenderTracker*, Transform2d*>> desiredTracker = ECS::findOneOf<TilemapRenderTracker, Transform2d>();
        if (!desiredTracker.has_value()) return;
        const Transform2d* trackerTrans = std::get<1>(desiredTracker.value());

        const Vec2ui relativeTracker = map.getClosestHashLocation(trans.position, trackerTrans->position);
        const std::int32_t drawRadius = 2; // todo change dynamically with camera zoom

        for (std::size_t layer = 0; layer < map.getLayerCount(); ++layer) {
            for (std::int32_t x = -drawRadius + 1; x < drawRadius; ++x) {
                for (std::int32_t y = -drawRadius + 1; y < drawRadius; ++y) {

                    const Vec2 desiredPosition = {static_cast<float>(relativeTracker.x) + x, static_cast<float>(relativeTracker.y) + y};
                    if (desiredPosition.x < 0 || desiredPosition.y < 0) continue;

                    const auto desiredHash = Vec2ui(desiredPosition);

                    for (const auto&[sprite, position] : map.getTileView(layer, desiredHash)) {
                        const auto pos = map.toWorldPosition(trans.position, position);
                        sprite.texture->renderEx(pos, sprite.offset, 0.0f, 1.0f, sprite.tint);
                    }
                }
            }

        }
    }

    inline void registerAll() {
        Universe::onEarlyRender2d.registerSystem<Tilemap, Transform2d>(renderTilemapsInFull);
    }
}

#endif //GAME_TILEMAP_H