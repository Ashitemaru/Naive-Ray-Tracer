#pragma once

#include "geometry/object3d.hpp"

class Sphere : public Object3D {
private:
    Vector3f center; // Center of the sphere
    double radius; // Radius of the sphere

public:
    Sphere() {
        center = Vector3f(0., 0., 0.);
        radius = 1.;
    }

    Sphere(const Vector3f &_c, double _r, Material *m)
        : Object3D(m) {
        center = _c;
        radius = _r;
    }

    virtual ~Sphere() override = default;

    virtual bool intersect(const Ray &r, Hit &h, double tmin) const override {
        Vector3f l = center - r.o;
        double r_sqr = radius * radius;

        double tp = Vector3f::dot(l, r.d.normalized());
        double d_sqr = l.squaredLength() - tp * tp;

        if (d_sqr > r_sqr) return false;

        double t = 0.;
        if (l.squaredLength() > r_sqr) {
            if (tp <= 0) return false;
            t = tp - sqrt(r_sqr - d_sqr);
        } else {
            t = tp + sqrt(r_sqr - d_sqr);
        }

        if (t < tmin || t >= h.t) return false;

        h.set(t, material, HitSurface {
            r.at(t), (r.at(t) - center).normalized()
        });
        return true;
    }

    virtual std::pair<HitSurface, double> samplePoint(RandomEngine &reng) const override {
		float phi = 2 * M_PI * reng.getUniformDouble(0, 1);
		float z = 2 * reng.getUniformDouble(0, 1) - 1;
		Vector3f normal(std::sqrt(1 - z * z) * std::cos(phi), std::sqrt(1 - z * z) * std::sin(phi), z);
		Vector3f pos = center + normal * radius;

        return std::make_pair(HitSurface { pos, (pos - center).normalized() }, 1. / (4 * M_PI * radius * radius));
	}
};