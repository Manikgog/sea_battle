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

inline int getRandomNumber(int start_number, int end_number) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(start_number, end_number);
    return dis(gen);
}


#endif // UTILS_HPP
