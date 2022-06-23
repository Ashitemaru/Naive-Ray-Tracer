#pragma once

#include <vecmath.h>
#include <cassert>
#include <cmath>

class Trans {
public:
    static Vector3f reflect(const Vector3f &i, const Vector3f &n) {
        return -i + 2 * Vector3f::dot(i, n) * n;
    }

    static Vector3f refract(const Vector3f &i, const Vector3f &n, double nin, double nout) {
        double mu = nin / nout;
        double x = Vector3f::dot(i, n);
        assert(x >= 0);

        return mu * mu * (1 - x * x) >= 1
            ? Vector3f::ZERO
            : mu * (n * x - i) - sqrt(1 - mu * mu * (1 - x * x)) * n;
    }

    static Vector3f generateVertical(const Vector3f &v) {
        if (std::abs(v[0]) <= std::abs(v[1]) && std::abs(v[0]) <= std::abs(v[2]))
            return Vector3f(0, v[2], -v[1]).normalized();
        else if (std::abs(v[1]) <= std::abs(v[0]) && std::abs(v[1]) <= std::abs(v[2]))
            return Vector3f(v[2], 0, -v[0]).normalized();
        else
            return Vector3f(v[1], -v[0], 0).normalized();
    }

    static Vector3f worldToLocal(
        const Vector3f &x,
        const Vector3f &y,
        const Vector3f &z,
        const Vector3f &world_cor
    ) {
        return Vector3f(
            Vector3f::dot(world_cor, x),
            Vector3f::dot(world_cor, y),
            Vector3f::dot(world_cor, z)
        );
    }

    static Vector3f localToWorld(
        const Vector3f &x,
        const Vector3f &y,
        const Vector3f &z,
        const Vector3f &local_cor
    ) {
        return local_cor[0] * x + local_cor[1] * y + local_cor[2] * z;
    }
};