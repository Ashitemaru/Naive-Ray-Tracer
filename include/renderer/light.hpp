#pragma once

#include "renderer/ray.hpp"
#include "utils/random_engine.hpp"

#include <vecmath.h>

class Light {
public:
    Ray sampleRay(Vector3f &power, double &pdf, RandomEngine &reng) {
        // TODO
    }
};