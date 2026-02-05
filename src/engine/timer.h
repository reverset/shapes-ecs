#ifndef GAME_TIMER_H
#define GAME_TIMER_H

struct Duration {
    std::uint64_t millis = 0;

    static constexpr Duration ofSeconds(const double seconds) {
        return Duration(static_cast<std::uint64_t>(seconds * 1000));
    }

    [[nodiscard]] constexpr double toSeconds() const {
        return static_cast<double>(millis) / 1000.0;
    }

    [[nodiscard]] constexpr std::uint64_t toMillis() const {
        return millis;
    }

    constexpr explicit Duration() = default;

    constexpr explicit Duration(const std::uint64_t millis) {
        this->millis = millis;
    }

    constexpr Duration operator+(const Duration other) const {
        return Duration(millis + other.millis);
    }

    constexpr Duration operator-(const Duration other) const {
        return Duration(millis - other.millis);
    }
};

// class GameTimer {
//     std::function<double()> timeFunc;
//     double startTime = -1;
//
//     public:
//     static GameTimer ofAppTime() {
//         return GameTimer(GetTime);
//     }
//
//     static GameTimer ofGameTime() {
//         return GameTimer(Universe::getGameTime);
//     }
//
//     explicit GameTimer(const std::function<float()> &timeSupplier) {
//       timeFunc = timeSupplier;
//     }
//
//     void reset() {
//         startTime = timeFunc();
//     }
//
//     void stop() {
//         startTime = -1;
//     }
//
//     [[nodiscard]] double getElapsed() const {
//         return timeFunc() - startTime;
//     }
//
//     [[nodiscard]] bool hasElapsed(const double time) const {
//         if (!isRunning()) return false;
//         return time < getElapsed();
//     }
//
//     bool hasElapsedAdvance(const double time) {
//         const bool el = hasElapsed(time);
//         if (el) reset();
//         return el;
//     }
//
//     [[nodiscard]] bool isRunning() const {
//         return startTime >= 0;
//     }
// };

#endif //GAME_TIMER_H