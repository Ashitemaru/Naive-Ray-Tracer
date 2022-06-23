#pragma once

#include "geometry/object3d.hpp"

class Plane : public Object3D {
private:
    Vector3f n; // Normal vector
    double d;

    bool textured;
    Vector3f text[2]; // Texture vectors
    Vector3f origin; // Origin coordination of texture

public:
    Plane() {
        this->n = Vector3f(0.0, 0.0, 0.0);
        this->d = 0.0;
        this->textured = false;
    }

    Plane(const Vector3f &_n, double _d, Material *_m)
        : Object3D(_m) {
        this->n = _n.normalized();
        this->d = _d;
        this->textured = false;
    }

    virtual ~Plane() override = default;

    virtual bool intersect(const Ray &r, Hit &h, double tmin) const override {
        double t = (d - Vector3f::dot(r.o, n)) / Vector3f::dot(r.d, n);
        if (t < tmin || t > h.t)
            return false;

        if (textured && material->textured()) {
            Vector3f pos = r.at(t) - this->origin;
            double e =
                text[0].squaredLength() * text[1].squaredLength() -
                Vector3f::dot(text[0], text[1]) * Vector3f::dot(text[0], text[1]);
            double s1 =
                Vector3f::dot(pos, text[0]) * text[1].squaredLength() -
                Vector3f::dot(text[0], text[1]) * Vector3f::dot(pos, text[1]);
            double s2 =
                Vector3f::dot(pos, text[1]) * text[0].squaredLength() -
                Vector3f::dot(text[0], text[1]) * Vector3f::dot(pos, text[0]); 
            h.set(t, material, HitSurface(r.at(t), n, n, Vector2f(s1 / e, s2 / e), true));
        } else {
            h.set(t, material, HitSurface(r.at(t), n));
        }
        return true;
    }

    virtual std::pair<HitSurface, double> samplePoint(RandomEngine &reng) const override {
		return std::make_pair(HitSurface { Vector3f::ZERO, n }, -1.);
	}

    // TODO: Understand here, texture related topics
    void addTexture(const Vector3f &e0, const Vector3f &e1, const Vector3f &o) {
        this->origin = o + n * (d - Vector3f::dot(n, o));
        this->text[0] = e0 - n * Vector3f::dot(n, e0);
        this->text[1] = e1 - n * Vector3f::dot(n, e1);
        this->textured = true;
    }
};