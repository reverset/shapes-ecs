#ifndef GAME_TILEMAP_H
#define GAME_TILEMAP_H

#include "vector"
#include "raylib.h"

#include "ecs.h"
#include "../engine/rendering.h"
#include "vec.h"

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
    std::vector<std::unordered_map<Vec2ui, RenderTexture2D, SpatialHash>> cachedTiles;

    std::uint32_t width;
    std::uint32_t height;

    float scalingFactor;
    float inverseScalingFactor;

    [[nodiscard]] static Logging::Logger& getLogger() {
        static auto logger = LOGGER(Tilemap);
        return logger;
    }

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
            cachedTiles.emplace_back();
        }
    }

    [[nodiscard]] std::uint32_t getWidth() const {
        return width;
    }

    [[nodiscard]] std::uint32_t getHeight() const {
        return height;
    }

    [[nodiscard]] bool isSpaceCached(const std::size_t layer, const Vec2ui hash) const {
        return cachedTiles.at(layer).contains(hash);
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

    [[nodiscard]] RenderTexture2D& getCachedTileTexture(const std::size_t layer, const Vec2ui hash) {
        return cachedTiles.at(layer).at(hash);
    }

    void preprocessSpace(const std::size_t layer, const Vec2ui hash) {
        if (isSpaceCached(layer, hash)) {
            UnloadRenderTexture(cachedTiles.at(layer).at(hash));
            cachedTiles.at(layer).erase(hash);
        }

        const RenderTexture2D tile = LoadRenderTexture(static_cast<int>(static_cast<float>(xHashFactor) * scalingFactor),
                                                       static_cast<int>(static_cast<float>(yHashFactor) * scalingFactor));
        std::size_t drawCalls = 0;
        BeginTextureMode(tile);
        ClearBackground(BLACK);

        for (const auto& [sprite, position] : getTileView(layer, hash)) {
            const auto pos = Vec2{position % Vec2ui{xHashFactor, yHashFactor}} * scalingFactor;

            const auto texture = sprite.texture->getTexture().value();
            const auto posFixedForCenter = pos + Vec2{static_cast<float>(texture.width)/2.0f, static_cast<float>(texture.height)/2.0f};

            sprite.texture->renderEx(posFixedForCenter, sprite.offset, 0.0f, 1.0f, sprite.tint);
            drawCalls++;
        }
        EndTextureMode();

        cachedTiles.at(layer)[hash] = tile;

        getLogger().log("Cached %zu draw calls.", drawCalls);
    }

    void unloadAllCachedTextures() const {
        for (std::size_t i = 0; i < getLayerCount(); ++i) {
            for (const auto& tex : cachedTiles.at(i) | std::views::values) {
                UnloadRenderTexture(tex);
            }
        }
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

    [[nodiscard]] float getScalingFactor() const {
        return scalingFactor;
    }

    void insertTile(const std::size_t layer, const Tile& tile) {
        auto& map = getLayer(layer);

        const auto ix = tile.position.x / xHashFactor;
        const auto iy = tile.position.y / yHashFactor;

        const Vec2ui hash{ix, iy};
        map[hash].push_back(tile);
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

    void cacheAll() {
        for (std::size_t i = 0; i < getLayerCount(); ++i) {
            for (const auto hash : getLayerView(i) | std::views::keys) {
                getLogger().log("Caching tile textures. Layer=%zu, hash=%s", i, hash.toString().c_str());
                preprocessSpace(i, hash);
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

    inline void unloadRenderTexturesInTilemaps(const Entity, const Tilemap& map) {
        map.unloadAllCachedTextures();
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

                    const auto desiredHash = static_cast<Vec2ui>(desiredPosition);

                    if (map.isSpaceCached(layer, desiredHash)) {
                        const RenderTexture2D& tex = map.getCachedTileTexture(layer, desiredHash);
                        const auto texturePosition = trans.position +
                            Vec2{desiredHash} *
                                Vec2::fromInts(tex.texture.width, tex.texture.height);

                        const auto width = static_cast<float>(tex.texture.width);
                        const auto height = static_cast<float>(tex.texture.height);

                        DrawTexturePro(
                            tex.texture,
                            {0, 0, width, -height},
                            {texturePosition.x, texturePosition.y, width, height},
                            {0, 0},
                            0.0f,
                            WHITE
                        );
                        // DrawTexture(tex.texture, texturePosition.xInt(), texturePosition.yInt(), WHITE);
                    } else {
                        for (const auto&[sprite, position] : map.getTileView(layer, desiredHash)) {
                            const auto pos = map.toWorldPosition(trans.position, position);
                            sprite.texture->renderEx(pos, sprite.offset, 0.0f, 1.0f, sprite.tint);
                        }
                    }
                }
            }

        }
    }

    inline void registerAll() {
        // Universe::onEarlyRender2d
        //     .registerSystem<Tilemap, Transform2d>(renderTilemapsInFull);

        Universe::onEarlyRender2d
            .registerSystem<Tilemap, Transform2d>(renderTilemapsChunked);

        Universe::onDeInit
            .registerSystem<Tilemap>(unloadRenderTexturesInTilemaps);
    }
}

#endif //GAME_TILEMAP_H