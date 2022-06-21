#pragma once

#include <cmath>
#include <cassert>

class Vector3 {
private:
    float x, y, z;

public:
    Vector3(float _x, float _y, float _z)
        : x(_x), y(_y), z(_z) { }

    static Vector3 zero() {
        return Vector3(0, 0, 0);
    }

    Vector3 operator+(const Vector3 &r) {
        return Vector3(x + r.x, y + r.y, z + r.z);
    }

    Vector3 operator-(const Vector3 &r) {
        return Vector3(x - r.x, y - r.y, z - r.z);
    }

    Vector3 operator-() {
        return Vector3(-x, -y, -z);
    }

    Vector3 operator*(float a) {
        return Vector3(x * a, y * a, z * a);
    }

    Vector3 operator*(const Vector3 &r) {
        return Vector3(x * r.x, y * r.y, z * r.z);
    }

    Vector3 operator/(float a) {
        return (*this) * (1.0 / a);
    }

    Vector3 operator/(const Vector3 &r) {
        return Vector3(x / r.x, y / r.y, z / r.z);
    }

    float &operator[](int index) {
        if (index == 0) return x;
        else if (index == 1) return y;
        else if (index == 2) return z;
        else assert(false);
    }

    bool operator==(const Vector3 &r) {
        return (x == r.x) && (y == r.y) && (z == r.z);
    }

    bool operator!=(const Vector3 &r) {
        return !((*this) == r);
    }

    Vector3 &operator+=(const Vector3 &r) {
        x += r.x;
        y += r.y;
        z += r.z;
        return *this;
    }

    Vector3 &operator-=(const Vector3 &r) {
        x -= r.x;
        y -= r.y;
        z -= r.z;
        return *this;
    }

    Vector3 &operator*=(const Vector3 &r) {
        x *= r.x;
        y *= r.y;
        z *= r.z;
        return *this;
    }

    Vector3 &operator/=(const Vector3 &r) {
        x /= r.x;
        y /= r.y;
        z /= r.z;
        return *this;
    }

    float dot(const Vector3 &r) {
        return x * r.x + y * r.y + z * r.z;
    }

    Vector3 cross(const Vector3 &r) {
        return Vector3(
            y * r.z - z * r.y,
            z * r.x - x * r.z,
            x * r.y - y * r.x
        );
    }

    float len() {
        return sqrtf(this->dot(*this));
    }

    Vector3 norm() {
        return (*this) / len();
    }
};
