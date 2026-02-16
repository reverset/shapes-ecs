#ifndef GAME_LAYERSANDMASKS_H
#define GAME_LAYERSANDMASKS_H

#include <cstdint>
#include <limits>

namespace BitLayers {
    using Type = std::uint32_t;

    [[nodiscard]] consteval Type bit(const std::int32_t where) {
        return 1u << where;
    }

    constexpr Type NONE = 0;
    constexpr Type PLAYER_LAYER = bit(1);
    constexpr Type ENEMY_LAYER = bit(2);

    constexpr Type ALL = std::numeric_limits<Type>::max();

    constexpr bool isBitIn(const std::int32_t mask, const Type layer) {
        return (1u << mask) & layer;
    }
}

#endif //GAME_LAYERSANDMASKS_H