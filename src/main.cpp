#include <cstdio>

#include <SDL.h>

int main(int argc, char** argv) {
    std::printf("Hello, World!\n");
    for(int i = 0; i < argc; ++i) {
        std::printf("Arg %d: %s", i, argv[i]);
    }
    return 0;
}
