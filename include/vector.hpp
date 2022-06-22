#pragma once

#include <cmath>
#include <cassert>

template <typename T, int dim>
class Vector {
private:
    T x[dim];

public:
    Vector(T _x, T _y, T _z)
        : x(_x), y(_y), z(_z) { }

    Vector<T, dim> operator+(const Vector<T, dim> &r) {
        return Vector<T, dim>(x + r.x, y + r.y, z + r.z);
    }

    Vector<T, dim> operator-(const Vector<T, dim> &r) {
        return Vector<T, dim>(x - r.x, y - r.y, z - r.z);
    }

    Vector<T, dim> operator-() {
        return Vector<T, dim>(-x, -y, -z);
    }

    Vector<T, dim> operator*(T a) {
        return Vector<T, dim>(x * a, y * a, z * a);
    }

    Vector<T, dim> operator*(const Vector<T, dim> &r) {
        return Vector<T, dim>(x * r.x, y * r.y, z * r.z);
    }

    Vector<T, dim> operator/(T a) {
        return (*this) * (1.0 / a);
    }

    Vector<T, dim> operator/(const Vector<T, dim> &r) {
        return Vector<T, dim>(x / r.x, y / r.y, z / r.z);
    }

    T &operator[](int index) {
        if (index >= 0 && index < dim)
            return x[index];
        else
            assert(false);
    }

    bool operator==(const Vector<T, dim> &r) {
        for (int i = 0; i < dim; ++i)
            if (x[i] != r.x[i])
                return false;
        return true;
    }

    bool operator!=(const Vector<T, dim> &r) {
        return !((*this) == r);
    }

    Vector<T, dim> &operator+=(const Vector<T, dim> &r) {
        x += r.x;
        y += r.y;
        z += r.z;
        return *this;
    }

    Vector<T, dim> &operator-=(const Vector<T, dim> &r) {
        x -= r.x;
        y -= r.y;
        z -= r.z;
        return *this;
    }

    Vector<T, dim> &operator*=(const Vector<T, dim> &r) {
        x *= r.x;
        y *= r.y;
        z *= r.z;
        return *this;
    }

    Vector<T, dim> &operator/=(const Vector<T, dim> &r) {
        x /= r.x;
        y /= r.y;
        z /= r.z;
        return *this;
    }

    T dot(const Vector<T, dim> &r) {
        T result = 0;
        for (int i = 0; i < dim; ++i)
            result += x[i] * r.x[i];
        return result;
    }

    Vector cross(const Vector &r) {
        return Vector(
            y * r.z - z * r.y,
            z * r.x - x * r.z,
            x * r.y - y * r.x
       );
    }

    T len() {
        return sqrtf(this->dot(*this));
    }

    Vector norm() {
        return (*this) / len();
    }
};
