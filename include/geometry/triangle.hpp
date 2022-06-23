#pragma once

#include "geometry/object3d.hpp"

/**
 * @ref: https://github.com/Numendacil/Graphics/blob/master/include/triangle.hpp
 * Sorry, my geometry is too bad to write such an excellent class,
 * so I need to deirectly use the one created by other students and try my best to understand it.
 */
class Triangle : public Object3D {
private:
    Vector3f normal[3];
    Vector3f geoNormal;
    Vector3f vertices[3];
    Vector2f cord[3];
    bool textured;

public:
    Triangle() = delete;

    Triangle(const Vector3f &a, const Vector3f &b, const Vector3f &c, Material *m)
        : Object3D(m) {
        this->vertices[0] = a;
        this->vertices[1] = b;
        this->vertices[2] = c;
        this->geoNormal = Vector3f::cross(b - a, c - a).normalized();
        this->normal[0] = this->geoNormal;
        this->normal[1] = this->geoNormal;
        this->normal[2] = this->geoNormal;
        this->textured = false;
    }

    void setNormal(const Vector3f &n0, const Vector3f &n1, const Vector3f &n2) {
        normal[0] = n0;
        normal[1] = n1;
        normal[2] = n2;
    }

    void setCord(const Vector2f &t0, const Vector2f &t1, const Vector2f &t2) {
        cord[0] = t0;
        cord[1] = t1;
        cord[2] = t2;
        textured = true;
    }

    bool intersect(const Ray &ray, Hit &hit, double tmin) const override {
        Vector3f e1 = vertices[0] - vertices[1];
        Vector3f e2 = vertices[0] - vertices[2];
        Vector3f s = vertices[0] - ray.o;

        double det1 = Matrix3f(ray.d, e1, e2).determinant();
        if (abs(det1) < 1e-6) return false;

        double t = Matrix3f(s, e1, e2).determinant() / det1;
        if (t < tmin || t >= hit.t) return false;

        double beta = Matrix3f(ray.d, s, e2).determinant() / det1;
        if (beta < 0 || beta > 1) return false;
        double gamma = Matrix3f(ray.d, e1, s).determinant() / det1;
        if (gamma < 0 || gamma > 1 || beta + gamma > 1) return false;

        Vector3f norm = (1 - beta - gamma) * normal[0] + beta * normal[1] + gamma * normal[2];
        Vector2f tex;
        if (textured && material->textured())
            tex = (1 - beta - gamma) * cord[0] + beta * cord[1] + gamma * cord[2];
        
        hit.set(t, material, HitSurface(ray.at(t), norm, geoNormal, tex, textured && material->textured()));
        return true;
    }

    std::pair<HitSurface, double> samplePoint(RandomEngine &reng) const override {
        double area = Vector3f::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]).length() / 2.;
        double pdf = 1. / area;
        double a = reng.getUniformDouble(0, 1);
        double b = reng.getUniformDouble(0, 1);
        if (a + b >= 1) {
            a = 1 - a;
            b = 1 - b;
        }

        Vector3f pos = (1 - a - b) * vertices[0] + a * vertices[1] + b * vertices[2];
        Vector3f norm = (1 - a - b) * normal[0] + a * normal[1] + b * normal[2];
        Vector2f tex;
        if (textured && material->textured())
            tex = (1 - a - b) * cord[0] + a * cord[1] + b * cord[2];

        return std::make_pair(
            HitSurface(pos, norm, geoNormal, tex, textured && material->textured()),
            pdf
        );
    }
};