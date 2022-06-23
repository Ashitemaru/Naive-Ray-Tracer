#pragma once

#include "renderer/ray.hpp"
#include "utils/random_engine.hpp"

#include <vecmath.h>

class Light {
public:
    Light() = default;

    virtual ~Light() = default;

    virtual Vector3f getIllumin(const Vector3f &dir) const = 0;
    virtual bool intersect(const Ray &r, Hit &h, double tmin) const = 0;
    virtual Ray sampleRay(Vector3f &power, double &pdf, RandomEngine &reng) const = 0;
};

class AreaLight {

};