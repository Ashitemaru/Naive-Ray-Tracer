#pragma once

#include <vecmath.h>

class Hit {
public:
    double t;
    Vector3f n; // The normal vec of the hit point
    Vector2f uv;

public:
};