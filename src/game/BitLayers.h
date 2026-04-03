#ifndef GAME_LAYERSANDMASKS_H
#define GAME_LAYERSANDMASKS_H

#include <cstdint>
#include <limits>

namespace BitLayers {
    using Type = std::uint32_t;

    [[nodiscard]] constexpr Type bit(const std::int32_t where) {
        return 1u << where;
    }

    [[nodiscard]] constexpr Type setBit(const Type value, const std::int32_t where, const bool on = true) {
        return (value & ~bit(where)) | (on ? bit(where) : 0);
    }

    constexpr Type NONE = 0;
    constexpr Type PLAYER_LAYER = bit(1);
    constexpr Type ENEMY_LAYER = bit(2);

    constexpr Type ALL = std::numeric_limits<Type>::max();

    constexpr bool checkMask(const Type mask, const Type layer) {
        return (mask & layer) != 0;
    }
}

#endif //GAME_LAYERSANDMASKS_H