#ifndef GAME_ANIM_H
#define GAME_ANIM_H

#include <functional>

#include "ecs.h"
#include "Universe.h"

template <typename T>
struct Tween {
    using Tweener = std::function<T(double)>;

    Tweener tweener;

    Timestamp startTime = Timestamp::longAgo();
    Duration desiredLength;

    explicit Tween(const Duration length, const Tweener& tweener)
        : tweener(tweener), desiredLength(length) {}

    void start() {
        startTime = Timestamp::now();
    }

    [[nodiscard]] bool hasStarted() const {
        return startTime != Timestamp::longAgo();
    }

    [[nodiscard]] bool isFinished() const {
        return hasStarted() && startTime.hasElapsed(desiredLength);
    }

    [[nodiscard]] std::optional<T> calculate() const {
        if (!hasStarted()) return std::nullopt;

        const double norm = startTime.normalizedElapsed(desiredLength);

        return tweener(norm);
    }

    void stop() {
        startTime = Timestamp::longAgo();
    }
};

namespace Tweeners {
    [[nodiscard]] constexpr std::function<double(double)> lerp(const double start, const double end) {
        return [=](const double dt) {
            return GameUtil::lerp(start, end, dt);
        };
    }
}

namespace Anim {
}

#endif
