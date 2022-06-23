#pragma once

#include "geometry.hpp"
#include <vecmath.h>

class Sphere : public Geometry {
private:
    Vector3f c; // Center of the sphere
    double r; // Radius of the sphere

public:
    virtual bool hit(const Ray &ray, Hit &h, double tmin) override {
        Vector3f oc_vec = c - ray.o;
        // H is the foot of vertical line from center to the ray
        double oh = Vector3f::dot(oc_vec, ray.d);
        double oc = oc_vec.length();
        double det = oh * oh + r * r - oc * oc;

        // The distance from center to the ray is greater than radius, not hit
        if (det < 0.0) {
            return false;
        } else {
            // P is the intersection point of ray & sphere
            double hp = sqrt(det);
        }
    }
};