#pragma once

#include "renderer/ray.hpp"
#include "renderer/hit.hpp"

class Geometry {
public:
    virtual bool hit(const Ray &, Hit &, double) = 0;
};