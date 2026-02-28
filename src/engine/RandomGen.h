#ifndef GAME_RANDOMGEN_H
#define GAME_RANDOMGEN_H

#include <cinttypes>

namespace RandomGen {
    void init();
    int random(int min, int max);
    double randomNormalized();
    float randomFloat(float min, float max);

    std::size_t randomSizet(std::size_t min, std::size_t max);
}


#endif //GAME_RANDOMGEN_H