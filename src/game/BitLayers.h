#ifndef GAME_LAYERSANDMASKS_H
#define GAME_LAYERSANDMASKS_H

#include <cstdint>
#include <limits>

namespace BitLayers {
    using Type = std::uint32_t;

    [[nodiscard]] consteval Type bit(const std::int32_t where) {
        return 0u | (1u << where);
    }

    constexpr Type NONE = 0;
    constexpr Type PLAYER_LAYER = bit(1);
    constexpr Type ENEMY_LAYER = bit(2);

    constexpr Type ALL = std::numeric_limits<Type>::max();
}

#endif //GAME_LAYERSANDMASKS_H