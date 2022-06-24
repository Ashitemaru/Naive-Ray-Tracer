#pragma once

#include "geometry/rectangle.hpp"

#include <iostream>

class BBox : public Rectangle {
public:
    BBox() = delete;

    BBox(const Vector3f &urf, const Vector3f &llb)
        : Rectangle(urf, llb, nullptr) { }

    bool intersect(const Ray &ray, Hit &hit, double tmin) const override {
        tmin = 0.;

        bool inside = true;
        enum {LEFT, RIGHT, MIDDLE} pos[3];

        Vector3f candidate;
        Vector3f origin = ray.o, dir = ray.d.normalized();
        double length = ray.d.length();

        for (int i = 0; i < 3; i++) {
            if (origin[i] < LLB[i] - std::max(0., tmin * dir[i])) {
                pos[i] = LEFT;
                inside = false;
                candidate[i] = LLB[i];
            } else if (origin[i] > URF[i] + std::max(0., -tmin * dir[i])) {
                pos[i] = RIGHT;
                inside = false;
                candidate[i] = URF[i];
            } else {
                pos[i] = MIDDLE;
                candidate[i] = dir[i] > 0 ? URF[i] : LLB[i];
            }
        }

        if (!inside) {
            double tmax = 0.;
            int maxIdx = 0;
            for (int i = 0; i < 3; i++)
                if (pos[i] != MIDDLE && std::abs(dir[i]) > 1e-6)
                    if (tmax < (candidate[i] - origin[i]) / dir[i]) {
                        tmax = (candidate[i] - origin[i]) / dir[i];
                        maxIdx = i;
                    }

            if (tmax / length < tmin || tmax / length > hit.t)
                return false;

            Vector3f position = ray.at(tmax / length);
            Vector3f normal;

            for (int i = 0; i < 3; i++) {
                if (maxIdx != i) {
                    if (position[i] < LLB[i] || position[i] > URF[i])
                        return false;
                    else { }
                } else {
                    normal[i] = (pos[i] == RIGHT)? 1 : -1;
                }
            }

            hit.set(tmax / length, nullptr, {position, normal});
            return true;
        } else {
            double t = INFINITY;
            int minIdx = 0;
            for (int i = 0; i < 3; i++)
                if (std::abs(dir[i]) > 1e-6 && (candidate[i] - origin[i]) / dir[i] >= 0)
                    if (t > (candidate[i] - origin[i]) / dir[i]) {
                        t = (candidate[i] - origin[i]) / dir[i];
                        minIdx = i;
                    }

            if (t / length < tmin || t / length > hit.t)
                return false;

            Vector3f position = ray.at(t / length);
            Vector3f normal;
            normal[minIdx] = dir[minIdx] < 0 ? -1 : 1;

            hit.set(t / length, nullptr, {position, normal});
            return true;
        }
    }

    virtual std::pair<HitSurface, double> samplePoint(RandomEngine&reng) const override {
        return std::make_pair(HitSurface {
            Vector3f::ZERO,
            Vector3f::ZERO,
        }, -1.);
    }

    Vector3f getCenter() const { return (URF + LLB) / 2.; }

    BBox* getChild(int octant) const {
        Vector3f center = getCenter();
        assert(octant >= 0 && octant < 8);

        Vector3f corner;
        for (int i = 0; i < 3; i++)
            corner[i] = ((octant >> (2 - i)) & 0x1) ? URF[i] : LLB[i];
        return new BBox(center, corner);
    }

    bool in(const Vector3f &v) const {
        for (int i = 0; i < 3; i++)
            if (v[i] < LLB[i] || v[i] > URF[i])
                return false;
        return true;
    }

    bool triIntersectBox(const Vector3f &a, const Vector3f &b, const Vector3f &c) const {
        bool result = true;
        for (int i = 0; i < 3; i++) {
            double max_ = std::max(a[i], std::max(b[i], c[i])) + 1e-6;
            double min_ = std::min(a[i], std::min(b[i], c[i])) - 1e-6;
            result &= (min_ < URF[i]) && (max_ > LLB[i]);
        }
        return result;
    }
};