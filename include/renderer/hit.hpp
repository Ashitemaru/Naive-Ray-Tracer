#pragma once

#include <vecmath.h>

#include "renderer/material.hpp"

struct HitSurface {
    Vector3f position;
    Vector3f normal;
    Vector3f geoNormal;
    Vector2f cord;
    bool hasTexture;

    HitSurface() { }
    HitSurface(
        const Vector3f &pos,
        const Vector3f &norm,
        const Vector3f &geonorm = Vector3f::ZERO,
        const Vector2f &cord = Vector2f::ZERO,
        bool flag = false
    ) {
        this->position = pos;
        this->normal = norm;
        this->geoNormal = (geonorm == Vector3f::ZERO) ? norm : geonorm;
        this->cord = cord;
        this->hasTexture = flag;
    }
};

class Hit {
public:
    double t;
    Material *material;
    HitSurface surface;

public:
    Hit() {
        t = INFINITY;
        material = nullptr;
    }

    Hit(double _t, Material *_material, const HitSurface &_surface) {
        t = _t;
        material = _material;
        surface = _surface;
    }

    Hit(const Hit &h) = default;

    void set(double _t, Material *_material, const HitSurface &_surface) {
        t = _t;
        material = _material;
        surface = _surface;
    }

    ~Hit() = default;
};