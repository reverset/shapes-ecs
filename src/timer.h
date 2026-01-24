#ifndef GAME_TIMER_H
#define GAME_TIMER_H

#include "GameObject.h"
#include <functional>

#include "../Universe.h"

class GameTimer : public GameObject {
    std::function<double()> timeFunc;
    double startTime = -1;

    public:
    static GameTimer ofAppTime() {
        return GameTimer(GetTime);
    }

    static GameTimer ofGameTime() {
        return GameTimer(Universe::getGameTime);
    }

    explicit GameTimer(const std::function<float()> &timeSupplier) {
      timeFunc = timeSupplier;
    }

    void reset() {
        startTime = timeFunc();
    }

    void stop() {
        startTime = -1;
    }

    [[nodiscard]] double getElapsed() const {
        return timeFunc() - startTime;
    }

    [[nodiscard]] bool hasElapsed(const double time) const {
        if (!isRunning()) return false;
        return time < getElapsed();
    }

    bool hasElapsedAdvance(const double time) {
        const bool el = hasElapsed(time);
        if (el) reset();
        return el;
    }

    [[nodiscard]] bool isRunning() const {
        return startTime >= 0;
    }
};

#endif //GAME_TIMER_H