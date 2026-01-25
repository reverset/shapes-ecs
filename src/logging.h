#ifndef GAME_LOGGING_H
#define GAME_LOGGING_H

#include <iostream>
#include <chrono>
#include "util.h"

namespace Logging {
    inline bool loggingEnabled = true;

    template <typename ... Args>
    void log(const std::string& format, Args ... args) {
        if (!loggingEnabled) return;
        const auto now = std::chrono::system_clock::now();

        std::cout << '[' << now << "] [INFO] " << GameUtil::fmt(format, args...) << std::endl;
    }

    template <typename ... Args>
    void logWarn(const std::string& format, Args ... args) {
        if (!loggingEnabled) return;
        const auto now = std::chrono::system_clock::now();

        std::cerr << '[' << now << "] [WARN] " << GameUtil::fmt(format, args...) << std::endl;
    }
}

#endif //GAME_LOGGING_H