#ifndef GAME_FILES_H
#define GAME_FILES_H

#include <string>

#include "raylib.h"

namespace Files {
    [[nodiscard]] inline std::string path(const std::string& str) {
        const std::string& dir = GetApplicationDirectory();
        #ifndef NDEBUG
        return dir + "/../resources/" + str;
        #else
        return dir + "/resources/" + str;
        #endif
    }

    [[nodiscard]] inline std::string path(const char* str) {
        return path(std::string(str));
    }
}

#endif //GAME_FILES_H