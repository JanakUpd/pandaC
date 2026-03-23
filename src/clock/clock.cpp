//
// Created by janak on 23.03.2026.
//

#include "../../include/clock/clock.h"
std::chrono::time_point<std::chrono::steady_clock> clockTimer::now() {
    return std::chrono::high_resolution_clock::now();
}
void clockTimer::start() {
    start_time = now();
}
uint64_t clockTimer::getSeconds() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(now() - start_time).count();
}
uint64_t clockTimer::getMiliseconds() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(now() - start_time).count();
}
uint64_t clockTimer::getMicroseconds() {
    return std::chrono::duration_cast<std::chrono::microseconds>(now() - start_time).count();
}
uint64_t clockTimer::getNanoseconds() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now() - start_time).count();
}