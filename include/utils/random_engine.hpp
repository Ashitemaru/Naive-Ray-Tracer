#pragma once

#include <random>
#include <vector>

/**
 * @ref: https://github.com/Numendacil/Graphics/blob/master/include/utils.hpp
 */
class RandomEngine {
private:
    std::mt19937 mt;
    unsigned int seed;

public:
    RandomEngine() {
        this->seed = std::random_device()();
        this->mt.seed(this->seed);
    }

    RandomEngine(unsigned int _seed) {
        this->seed = _seed;
        this->mt.seed(this->seed);
    }

    unsigned int getSeed() { return this->seed; }

    double getUniformDouble(double min, double max) {
        std::uniform_real_distribution<double> urd(min, max);
        return urd(this->mt);
    }

    std::vector<double> getUniformDoubleList(double min, double max, int n) {
        std::uniform_real_distribution<double> urd(min, max);
        std::vector<double> result;
        for (int i = 0; i < n; ++i)
            result.push_back(urd(this->mt));

        return result;
    }

    int getUniformInt(int min, int max) {
        std::uniform_int_distribution<int> uid(min, max);
        return uid(this->mt);
    }

    std::vector<int> getUniformIntList(int min, int max, int n) {
        std::uniform_int_distribution<int> uid(min, max);
        std::vector<int> result;
        for (int i = 0; i < n; ++i)
            result.push_back(uid(this->mt));

        return result;
    }
};