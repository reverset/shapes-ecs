#ifndef GAME_FILES_H
#define GAME_FILES_H

#include <string>

namespace Files {
    [[nodiscard]] inline std::string path(const std::string& str) {
        #ifndef NDEBUG
        return "../resources/" + str;
        #else
        return "resources/" + str;
        #endif
    }

    [[nodiscard]] inline std::string path(const char* str) {
        return path(std::string(str));
    }
}

#endif //GAME_FILES_H