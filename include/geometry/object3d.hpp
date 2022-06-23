#pragma once

#include "renderer/ray.hpp"
#include "renderer/hit.hpp"
#include "utils/random_engine.hpp"
#include "renderer/material.hpp"

class Object3D {
protected:
    Material *material;

public:
    Object3D(): material(nullptr) { }

    virtual ~Object3D() { delete material; }

    explicit Object3D(Material *_material)
        : material(_material) { }

    // Intersect Ray with this object. If hit, store information in hit structure.
    virtual bool intersect(const Ray &r, Hit &h, double tmin) const = 0;

    // Sample point on the object
    virtual std::pair<HitSurface, double> samplePoint(RandomEngine &reng) const = 0;
};