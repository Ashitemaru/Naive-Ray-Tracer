#pragma once

#include "renderer/ray.hpp"
#include "utils/random_engine.hpp"
#include "geometry/object3d.hpp"
#include "utils/trans.hpp"

#include <vecmath.h>

struct RaySampleResult {
    Ray ray;
    Vector3f power;
    double pdf;
};

class Light {
public:
    Light() = default;

    virtual ~Light() = default;

    virtual Vector3f getIllumin(const Vector3f &dir) const = 0;
    virtual bool intersect(const Ray &r, Hit &h, double tmin) const = 0;
    virtual RaySampleResult sampleRay(RandomEngine &reng) const = 0;
};

class AreaLight : public Light {
private:
    Object3D *obj;
    Vector3f power;

public:
    AreaLight() = delete;

    AreaLight(Object3D *o, const Vector3f &p)
        : obj(o), power(p) { }

    virtual ~AreaLight() override {
        delete obj;
    }

    virtual Vector3f getIllumin(const Vector3f &dir) const override {
        return power;
    }

    virtual bool intersect(const Ray &r, Hit &h, double tmin) const override {
        return obj->intersect(r, h, tmin);
    }

    virtual RaySampleResult sampleRay(RandomEngine &reng) const override {
        auto pair = obj->samplePoint(reng);
        HitSurface surface = pair.first;
        double pdf = pair.second;

        Vector3f x = surface.normal;
        Vector3f y = Trans::generateVertical(x);
        Vector3f z = Vector3f::cross(x, y).normalized();

        double phi = 2 * M_PI * reng.getUniformDouble(0, 1);
        double t = std::sqrt(reng.getUniformDouble(0, 1));
        pdf *= t / M_PI;

        Vector3f out = Vector3f(std::sqrt(1 - t * t) * std::cos(phi), std::sqrt(1 - t * t) * std::sin(phi), t);
        return RaySampleResult {
            .ray = Ray {
                surface.position,
                Trans::localToWorld(y, z, x, out),
            },
            .power = t * power,
            .pdf = pdf,
        };
    }
};

class PointLight : public Light {
private:
    Vector3f pos;
    Vector3f power;

public:
    PointLight() = delete;

    PointLight(const Vector3f &_pos, const Vector3f &_power)
        : pos(_pos), power(_power) { }

    virtual ~PointLight() override = default;

    virtual Vector3f getIllumin(const Vector3f &dir) const override {
        return power;
    }

    virtual bool intersect(const Ray &r, Hit &h, double tmin) const override {
        return false;
    }

    virtual RaySampleResult sampleRay(RandomEngine &reng) const override {
        double phi = 2 * M_PI * reng.getUniformDouble(0, 1);
        double z = reng.getUniformDouble(-1, 1);
        Vector3f out = Vector3f(
            std::sqrt(1 - z * z) * std::cos(phi),
            std::sqrt(1 - z * z) * std::sin(phi),
            z
        );

        return RaySampleResult {
            .ray = Ray { pos, out },
            .power = power,
            .pdf = 1. / (4. * M_PI),
        };
    }
};

class DirectedPointLight : public Light {
private:
    Vector3f pos;
    Vector3f direction;
    Vector3f power;
    double angle;

public:
    DirectedPointLight() = delete;

    DirectedPointLight(
        const Vector3f &_pos,
        const Vector3f &_direction,
        const Vector3f &_power,
        double _angle
    ) : pos(_pos), direction(_direction), power(_power), angle(_angle) { }

    virtual ~DirectedPointLight() override = default;

    virtual Vector3f getIllumin(const Vector3f &dir) const override {
        return power;
    }

    virtual bool intersect(const Ray &r, Hit &h, double tmin) const override {
        return false;
    }

    virtual RaySampleResult sampleRay(RandomEngine &reng) const override {
        double threshold = std::cos(angle);

        double phi = 2 * M_PI * reng.getUniformDouble(0, 1);
        double t = (1 - threshold) * reng.getUniformDouble(0, 1) + threshold;
        Vector3f out = Vector3f(
            std::sqrt(1 - t * t) * std::cos(phi),
            std::sqrt(1 - t * t) * std::sin(phi),
            t
        );

        Vector3f x = direction;
        Vector3f y = Trans::generateVertical(x);
        Vector3f z = Vector3f::cross(x, y);
        out = Trans::localToWorld(y, z, x, out);

        return RaySampleResult {
            .ray = Ray { pos, out },
            .power = power,
            .pdf = 1. / (2 * M_PI * (1 - threshold)),
        };
    }
};