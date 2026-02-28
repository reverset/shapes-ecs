
#include "RandomGen.h"

#include <random>
#include <chrono>
#include <optional>

namespace RandomGen {
    std::optional<std::minstd_rand> engine = std::nullopt;

    void init() {
        const unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        engine = std::minstd_rand(seed);
    }

    [[nodiscard]] int random(const int min, const int max) {
        std::uniform_int_distribution dis(min, max);
        return dis(engine.value());
    }

    [[nodiscard]] std::size_t randomSizet(const std::size_t min, const std::size_t max) {
        std::uniform_int_distribution dis(min, max);
        return dis(engine.value());
    }

    [[nodiscard]] float randomFloat(const float min, const float max) {
        std::uniform_real_distribution dis(min, max);
        return dis(engine.value());
    }

    [[nodiscard]] double randomNormalized() {
        std::uniform_real_distribution dis(0.0, 1.0);
        return dis(engine.value());
    }
}
