#ifndef GAME_TILEMAP_H
#define GAME_TILEMAP_H

#include <algorithm>
#include <queue>

#include "vector"
#include "raylib.h"

#include "ecs.h"
#include "vec.h"

#include "event.h"

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

    using TileHashMap = std::unordered_map<Vec2ui, std::vector<Tile>, SpatialHash>;

    std::vector<TileHashMap> tiles;
    std::vector<std::unordered_map<Vec2ui, RenderTexture2D, SpatialHash>> cachedTiles;

    std::unordered_map<Vec2ui, bool, SpatialHash> collidableTiles;

    std::uint32_t width;
    std::uint32_t height;

    float scalingFactor;
    float inverseScalingFactor;

    [[nodiscard]] static Logging::Logger& getLogger() {
        static auto logger = NEW_LOGGER(Tilemap);
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

    [[nodiscard]] std::vector<Vec2ui> getValidNeighbors(const Vec2ui point) const {
        std::vector<Vec2ui> res;
        for (std::int32_t x = static_cast<std::int32_t>(point.x) - 1; x < static_cast<std::int32_t>(point.x) + 2; ++x) {
            if (x < 0 && x >= width) continue;

            for (std::int32_t y = static_cast<std::int32_t>(point.y) - 1; y < static_cast<std::int32_t>(point.y) + 2; ++y) {
                if (y < 0 && y >= height) continue;

                res.push_back({
                    static_cast<std::uint32_t>(x),
                    static_cast<std::uint32_t>(y)
                });
            }
        }

        return res;
    }

    // This returns the path in reverse
    [[nodiscard]] std::vector<Vec2ui> calculatePath(const Vec2ui start, const Vec2ui end, const std::size_t layer) const {
        const std::function<std::size_t(Vec2ui)> heuristic = [end](const Vec2ui v) {
            return v.distanceSquared(end);
        };

        // todo use priority queue or min-heap for perfomance boost
        std::unordered_set<Vec2ui, SpatialHash> openSet = {};

        // TODO: perhaps dynamically load the openSet as it is requested?
        // or I could load all tiles in the spatial hashes between start and end
        // definitely dont copy ALL tiles, that would be insane.
        // const auto t = tiles.at(layer);

        std::unordered_map<Vec2ui, Vec2ui, SpatialHash> cameFrom;

        std::unordered_map<Vec2ui, std::size_t, SpatialHash> gScore;
        std::unordered_map<Vec2ui, std::size_t, SpatialHash> fScore;

        gScore[start] = 0;
        fScore[start] = heuristic(start);

        while (!openSet.empty()) {
            auto current = std::ranges::fold_left(openSet.begin(), openSet.end(), start, [&](Vec2ui l, Vec2ui r) {
                if (!fScore.contains(l)) {
                    fScore[l] = std::numeric_limits<std::size_t>::max();
                }
                if (!fScore.contains(r)) {
                    fScore[r] = std::numeric_limits<std::size_t>::max();
                }

                const auto lscore = fScore.at(l);
                const auto rscore = fScore.at(r);

                if (lscore < rscore)
                    return l;

                return r;
            });

            if (current == end) {
                std::vector<Vec2ui> path;
                path.reserve(cameFrom.size()); // over allocates

                while (cameFrom.contains(current)) {
                    current = cameFrom.at(current);
                    path.push_back(current);
                }

                return path;
            }

            openSet.erase(current);

            for (const auto neighbor : getValidNeighbors(current)) {
                const auto tentativeGscore = gScore.at(current) + 1; // replace 1 with 'the weight of the edge from current to neighbor'

                if (!gScore.contains(neighbor)) {
                    gScore[neighbor] = std::numeric_limits<std::size_t>::max();
                }

                if (tentativeGscore < gScore[neighbor]) {
                    cameFrom[neighbor] = current;
                    gScore[neighbor] = tentativeGscore;
                    fScore[neighbor] = tentativeGscore + heuristic(neighbor);
                    if (!openSet.contains(neighbor)) {
                        openSet.insert(neighbor);
                    }
                }
            }
        }

        return {};
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
        ClearBackground(Color{0, 0, 0, 1});

        for (const auto& [sprite, position] : getTileView(layer, hash)) {
            const auto pos = Vec2{position % Vec2ui{xHashFactor, yHashFactor}} * scalingFactor;

            const auto texture = sprite.texture->getTexture().value();
            const auto posFixedForCenter = pos + Vec2{static_cast<float>(texture.width)*0.5f, static_cast<float>(texture.height)*0.5f};

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

    [[nodiscard]] float getInverseScalingFactor() const {
        return inverseScalingFactor;
    }

    [[nodiscard]] bool checkCollisionAt(const Vec2ui point) const {
        if (!collidableTiles.contains(point)) return false;
        return collidableTiles.at(point);
    }

    void insertTile(const std::size_t layer, const Tile& tile, const bool collidable = false) {
        auto& map = getLayer(layer);

        const auto ix = tile.position.x / xHashFactor;
        const auto iy = tile.position.y / yHashFactor;

        const Vec2ui hash{ix, iy};
        map[hash].push_back(tile);

        if (collidable) {
            collidableTiles[tile.position] = true;
        }
    }

    void fillTile(const std::size_t layer, const Sprite& sprite, const Vec2ui start, const Vec2ui end, const bool collidable = false) {
        for (std::uint32_t x = start.x; x < end.x; ++x) {
            for (std::uint32_t y = start.y; y < end.y; ++y) {
                insertTile(layer, Tile {
                    .sprite = sprite,
                    .position = {x, y},
                }, collidable);
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

struct TilemapCollider : Component<TilemapCollider> {
    COMPONENT_STORAGE(TilemapCollider);

    Vec2 dimensions{0, 0};

    explicit TilemapCollider(const Vec2 dimensions) {
        this->dimensions = dimensions;
    }

    explicit TilemapCollider(const float width, const float height) {
        this->dimensions = {width, height};
    }
};

struct TilemapCollisionEvent : Event<TilemapCollisionEvent> {
    EVENT_STORAGE(TilemapCollisionEvent);

    Entity collided;
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

    inline void checkTilemapCollisions(const Entity theCollided, const TilemapCollider& collider, Transform2d& trans) {
        ECS::query<Tilemap, Transform2d>([&](const Entity, const Tilemap& map, const Transform2d& mapTrans) {
            const auto relPos = trans.position - mapTrans.position;
            if (relPos.x < 0 || relPos.y < 0) return;

            const auto scaledPos = relPos * map.getInverseScalingFactor();
            const auto index = static_cast<Vec2ui>(scaledPos);

            for (std::int32_t x = static_cast<std::int32_t>(index.x) - 1; x < static_cast<std::int32_t>(index.x) + 2; ++x) {
                if (x < 0) continue;

                for (std::int32_t y = static_cast<std::int32_t>(index.y) - 1; y < static_cast<std::int32_t>(index.y) + 2; ++y) {
                    if (y < 0) continue;

                    const auto point = Vec2ui{
                        static_cast<std::uint32_t>(x),
                        static_cast<std::uint32_t>(y),
                    };

                    // const Rectangle debug = {
                    //     static_cast<float>(x) * map.getScalingFactor(),
                    //     static_cast<float>(y) * map.getScalingFactor(),
                    //     16,
                    //     16,
                    // };
                    //
                    // DrawRectangleRec(debug, WHITE);

                    if (map.checkCollisionAt(point)) {
                        const Rectangle collisionRect = {
                            trans.position.x - collider.dimensions.x / 2,
                            trans.position.y - collider.dimensions.y / 2,
                            collider.dimensions.x,
                            collider.dimensions.y
                        };

                        const auto scaledPoint = point * map.getScalingFactor();
                        const auto tilePoint = Vec2{
                            mapTrans.position.x + static_cast<float>(scaledPoint.x),
                            mapTrans.position.y + static_cast<float>(scaledPoint.y),
                        };

                        const Rectangle tileRect = {
                            tilePoint.x,
                            tilePoint.y,
                            map.getScalingFactor(),
                            map.getScalingFactor(),
                        };

                        if (CheckCollisionRecs(collisionRect, tileRect)) {
                            Vec2 centeredTilePoint = {
                                tilePoint.x + map.getScalingFactor()*0.5f,
                                tilePoint.y + map.getScalingFactor()*0.5f
                            };

                            // math is hard
                            // https://www.reddit.com/r/raylib/comments/13b7p8r/how_do_i_create_collisions/
                            const Vec2 delta = trans.position - centeredTilePoint;

                            const Vec2 hs1 = { collisionRect.width*0.5f, collisionRect.height*0.5f };
                            const Vec2 hs2 = { map.getScalingFactor()*0.5f, map.getScalingFactor()*0.5f };

                            const float minX = hs1.x + hs2.x - std::abs(delta.x);
                            const float minY = hs1.y + hs2.y - std::abs(delta.y);

                            if (minX < minY) {
                                trans.position.x += std::copysign(minX, delta.x);
                            } else {
                                trans.position.y += std::copysign(minY, delta.y);
                            }

                            TilemapCollisionEvent event = {
                                .collided = theCollided,
                            };
                            event.send();
                        }
                    }
                }
            }


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

        // Universe::onUpdate
        //     .registerSystem<TilemapCollider, Transform2d>(checkTilemapCollisions);

        Universe::onEarlyRender2d
            .registerSystem<Tilemap, Transform2d>(renderTilemapsChunked);

        Universe::onRender2d // debugging
            .registerSystem<TilemapCollider, Transform2d>(checkTilemapCollisions);

        Universe::onDeInit
            .registerSystem<Tilemap>(unloadRenderTexturesInTilemaps);
    }
}

#endif //GAME_TILEMAP_H