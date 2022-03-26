#ifndef CUSTOM_STRUCT_H
#define CUSTOM_STRUCT_H
#include <utility>

struct imageSecretKey{
    std::pair<double, double> key1;
    std::pair<double, double> key2;
    std::pair<unsigned int, unsigned int> delta;
};
#endif