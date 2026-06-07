#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <random>
#include <algorithm>

template<typename T>
void shuffleVector(std::vector<T>& vec) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::shuffle(vec.begin(), vec.end(), gen);
}


#endif // UTILS_HPP
