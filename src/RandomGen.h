#ifndef GAME_RANDOMGEN_H
#define GAME_RANDOMGEN_H


namespace RandomGen {
    void init();
    int random(int min, int max);
    double randomNormalized();
    float randomFloat(float min, float max);
}


#endif //GAME_RANDOMGEN_H