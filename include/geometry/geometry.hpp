#pragma once

#include "ray.hpp"
#include "hit.hpp"

class Geometry {
public:
    virtual bool hit(const Ray &, Hit &, double) = 0;
};