#pragma once

#include <vecmath.h>

class Ray {
public:
    Vector3f o; // The origin of the ray
    Vector3f d; // The direction of the ray, normalized

public:
    Ray(Vector3f _o, Vector3f _d)
        : o(_o), d(_d.normalized()) { }
};