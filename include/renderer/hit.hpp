#pragma once

#include <vecmath.h>

#include "renderer/material.hpp"

struct HitSurface {
	Vector3f position;
	Vector3f normal;
	Vector3f geonormal;
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
		this->geonormal = (geonorm == Vector3f::ZERO) ? norm : geonorm;
		this->cord = cord;
		this->hasTexture = flag;
	}
};

class Hit {
public:
    double t;
    Vector3f n; // The normal vec of the hit point
    Vector2f uv;

public:
    Material *getMaterial() {
        // TODO
    }

    HitSurface &getSurface() {
        // TODO
    }
};