//
// Created by janak on 23.03.2026.
//

#ifndef PANDAC_CLOCK_H
#define PANDAC_CLOCK_H
#include <chrono>


class clockTimer {
    std::chrono::high_resolution_clock::time_point start_time;
    public:
        static std::chrono::time_point<std::chrono::steady_clock> now();
        void start();
        uint64_t getSeconds();
        uint64_t getMiliseconds();
        uint64_t getMicroseconds();
        uint64_t getNanoseconds();
    clockTimer() : start_time(std::chrono::high_resolution_clock::now()) {}
};


#endif //PANDAC_CLOCK_H