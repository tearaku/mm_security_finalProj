#include <iostream>
#include <cstdio>
#include <stdint.h>
 
int main(int argc, char** argv ) {
    int x = 1;
    char *y = (char*)&x;
    printf("%c\n",*y+48);
    std::cout << "sizeof(uint_fast32_t): " << sizeof(uint_fast32_t) << std::endl;
    std::cout << "sizeof(uint64_t): " << sizeof(uint64_t) << std::endl;
    return 0;
}