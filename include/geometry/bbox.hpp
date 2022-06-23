#pragma once

#include "geometry/rectangle.hpp"

class BBox : public Rectangle {
public:
    BBox() = delete;

    BBox(const Vector3f &urf, const Vector3f &llb)
        : Rectangle(urf, llb, nullptr) { }

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