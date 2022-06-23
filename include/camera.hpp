#pragma once

#include <vecmath.h>
#include <cmath>
#include <float.h>

#include "random_engine.hpp"
#include "ray.hpp"

class Camera {
protected:
    // Positional parameters
    Vector3f center;
    Vector3f direction; // Should be normalized
    Vector3f up; // Should be normalized
    Vector3f horizontal; // Should be normalized

    // Inner parameters
    int width;
    int height;
    double gamma;

public:
    Camera(
        const Vector3f &_center,
        const Vector3f &_direction,
        const Vector3f &_up,
        int _width,
        int _height,
        double _gamma
    ) {
        this->center = _center;
        this->direction = _direction.normalized();
        this->horizontal = Vector3f::cross(this->direction, _up).normalized();
		this->up = Vector3f::cross(this->horizontal, this->direction).normalized();
		this->width = _width;
		this->height = _height;
		this->gamma = _gamma;
    }

    virtual ~Camera() = default;
    virtual Ray sampleRay(int x, int y, RandomEngine &) const = 0;

    int getWidth() { return this->width; }
    int getHeight() { return this->height; }
    double getGamma() { return this->gamma; }
};

class PerspectiveCamera : public Camera {
protected:
    double fx, fy;

public:
    PerspectiveCamera(
        const Vector3f &_center,
        const Vector3f &_direction,
        const Vector3f &_up,
        int _width,
        int _height,
        double _gamma,
        double _angle // Specialized parameter for PerspectiveCamera
    ) : Camera(_center, _direction, _up, _width, _height, _gamma) {
        // Angle in radian
		this->fy = (double) _height / (2. * tan(_angle / 2.));
		this->fx = this->fy;
    }

    virtual Ray sampleRay(int x, int y, RandomEngine &reng) const override {
        double delta_x = reng.getUniformDouble(-0.5, 0.5);
        double delta_y = reng.getUniformDouble(-0.5, 0.5);

        Vector3f drc = Vector3f(
            (x + delta_x - .5 * this->width) / this->fx,
            (.5 * this->height - y + delta_y) / this->fy,
            1.
        ).normalized();
        Matrix3f rot = Matrix3f(this->horizontal, -this->up, this->direction);

        return Ray(this->center, rot * drc);
    }
};

class LenCamera : public Camera {
protected:
    double fx, fy;
    double aperture, f;

public:
    LenCamera(
        const Vector3f &_center,
        const Vector3f &_direction,
        const Vector3f &_up,
        int _width,
        int _height,
        double _gamma,
        double _angle, // Specialized parameter for LenCamera
        double _aperture, // Specialized parameter for LenCamera
        double _f // Specialized parameter for LenCamera
    ) : Camera(_center, _direction, _up, _width, _height, _gamma) {
        // Angle in radian
		this->fy = (double) _height / (2. * tan(_angle / 2.));
		this->fx = this->fy;
        this->aperture = _aperture;
        this->f = _f;
    }

    virtual Ray sampleRay(int x, int y, RandomEngine &reng) const override {
        double delta_x = reng.getUniformDouble(-0.5, 0.5);
        double delta_y = reng.getUniformDouble(-0.5, 0.5);

        // Sample a point (u, v) inside circle x^2 + y^2 = (1/2 * aperture)^2
        double u, v;
        do {
            u = reng.getUniformDouble(-1, 1);
            v = reng.getUniformDouble(-1, 1);
        } while (u * u + v * v > 1);
        u *= (.5 * this->aperture);
        v *= (.5 * this->aperture);

        Vector3f r = u * this->up + v * this->horizontal;
        Matrix3f rot = Matrix3f(this->horizontal, -this->up, this->direction);
        Vector3f drc = Vector3f(
            (x + delta_x - .5 * this->width) / this->fx,
            (.5 * this->height - y + delta_y) / this->fy,
            1.
        ).normalized();

        return Ray(this->center, (rot * drc - r).normalized() * this->f);
    }
};
