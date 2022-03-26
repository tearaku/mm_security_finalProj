#ifndef CHAOTIC_GEN_H
#define CHAOTIC_GEN_H
// Pseudo-random bit/byte generator
// TODO: change this to use chaotic map
#include <random>

class PRBG{
    std::minstd_rand0* generator;
public:
    std::uint_fast32_t getNextByte() {
        return (*generator)();
    }
    PRBG(std::minstd_rand0* devicePtr, std::int32_t key) {
        generator = devicePtr;
    }
    PRBG() {
        generator = nullptr;
    }
};

#endif