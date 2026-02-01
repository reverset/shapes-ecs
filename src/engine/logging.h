#ifndef GAME_LOGGING_H
#define GAME_LOGGING_H

#include <iostream>
#include <chrono>
#include "util.h"

#define NEW_LOGGER(name) Logging::Logger(#name)

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

    template <typename ... Args>
    void logError(const std::string& format, Args ... args) {
        if (!loggingEnabled) return;
        const auto now = std::chrono::system_clock::now();

        std::cerr << '[' << now << "] [ERR!] " << GameUtil::fmt(format, args...) << std::endl;
    }

    class Logger {
        std::string name;

    public:
        explicit Logger(const std::string& name) {
            this->name = name;
        }

        template <typename ... Args>
        void log(const std::string& format, Args ... args) {
            Logging::log("[" + name + "] " + format, args...);
        }

        template <typename ... Args>
        void logWarn(const std::string& format, Args ... args) {
            Logging::logWarn("[" + name + "] " + format, args...);
        }

        template <typename ... Args>
        void logError(const std::string& format, Args ... args) {
            Logging::logError("[" + name + "] " + format, args...);
        }
    };
}

#endif //GAME_LOGGING_H