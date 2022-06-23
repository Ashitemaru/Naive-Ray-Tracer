#pragma once

#include "geometry/object3d.hpp"

/**
 * @ref: https://github.com/Numendacil/Graphics/blob/master/include/rectangle.hpp
 * Sorry, my geometry is too bad to write such an excellent class,
 * so I need to deirectly use the one created by other students and try my best to understand it.
 */
class Rectangle : public Object3D {
private:
    Vector3f URF; // Up Right Front
    Vector3f LLB; // Low Left Behind

    Vector2f getUV(const Vector3f& p, int face) const {
        Vector3f size = this->URF - this->LLB;
        Vector3f point = p - this->LLB;
        Vector2f uv;
        assert(face >= 0 && face < 6);

        switch (face) {
            case 0:        // +x
                uv[0] = point[1] / (2. * (size[0] + size[1]));
                uv[1] = (point[2] + size[0]) / (2. * size[0] + size[2]);
                break;
            case 1:        // -x
                uv[0] = (2. * size[1] + size[0] - point[1]) / (2. * (size[0] + size[1]));
                uv[1] = (point[2] + size[0]) / (2. * size[0] + size[2]);
                break;
            case 2:        // +y
                uv[0] = (size[1] + size[0] - point[0]) / (2. * (size[0] + size[1]));
                uv[1] = (point[2] + size[0]) / (2. * size[0] + size[2]);
                break;
            case 3:        // -y
                uv[0] = (2. * size[1] + size[0] + point[0]) / (2. * (size[0] + size[1]));
                uv[1] = (point[2] + size[0]) / (2. * size[0] + size[2]);
                break;
            case 4:        // +z
                uv[0] = point[1] / (2. * (size[0] + size[1]));
                uv[1] = 1 - point[0] / (2. * size[0] + size[2]);
                break;
            case 5:        // -z
                uv[0] = point[1] / (2. * (size[0] + size[1]));
                uv[1] = point[0] / (2. * size[0] + size[2]);
                break;
        }
        return uv;
    }

public:
    Rectangle() = delete;

    Rectangle(const Vector3f &urf, const Vector3f &llb, Material *m)
        : Object3D(m) {
        this->URF = urf;
        this->LLB = llb;
        for (int i = 0; i < 3; i++)
            if (this->URF[i] < this->LLB[i])
                std::swap(this->URF[i], this->LLB[i]);
    }

    bool intersect(const Ray &ray, Hit &hit, double tmin) const override {
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

            if (this->material->textured()) {
                int face = maxIdx * 2 + (normal[maxIdx] > 0 ? 0 : 1);
                hit.set(
                    tmax / length,
                    this->material,
                    HitSurface(position, normal, normal, getUV(position, face), true) 
                );
            } else {
                hit.set(tmax / length, this->material, {position, normal});
            }
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

            if (this->material->textured()) {
                int face = minIdx * 2 + ((normal[minIdx] > 0)? 0 : 1);
                hit.set(
                    t / length,
                    this->material,
                    HitSurface(position, normal, normal, getUV(position, face), true)
                );
            } else {
                hit.set(t / length, this->material, {position, normal});
            }
            return true;
        }
    }

    std::pair<HitSurface, double> samplePoint(RandomEngine&reng) const override {
        double areaXY, areaYZ, areaZX;
        areaXY = (URF[0] - LLB[0]) * (URF[1] - LLB[1]);
        areaYZ = (URF[1] - LLB[1]) * (URF[2] - LLB[2]);
        areaZX = (URF[2] - LLB[2]) * (URF[0] - LLB[0]);
        double pdf = 1. / (areaXY + areaYZ + areaZX) / 2.;

        double face = reng.getUniformDouble(0, 1) * (areaXY + areaYZ + areaZX);
        if (face < areaXY) {
            bool which = face < (areaXY / 2.);
            return std::make_pair(HitSurface {
                Vector3f(
                    LLB[0] + reng.getUniformDouble(0, 1) * (URF[0] - LLB[0]),
                    LLB[1] + reng.getUniformDouble(0, 1) * (URF[1] - LLB[1]),
                    which ? LLB[2] : URF[2]
                ),
                Vector3f(0, 0, which ? -1 : 1)
            }, pdf);
        } else if (face < areaXY + areaYZ) {
            bool which = face - areaXY < (areaYZ / 2.);
            return std::make_pair(HitSurface {
                Vector3f(
                    which ? LLB[0] : URF[0],
                    LLB[1] + reng.getUniformDouble(0, 1) * (URF[1] - LLB[1]),
                    LLB[2] + reng.getUniformDouble(0, 1) * (URF[2] - LLB[2])
                ),
                Vector3f(which ? -1 : 1, 0, 0)
            }, pdf);
        } else {
            bool which = face - areaXY - areaZX < (areaZX / 2.);
            return std::make_pair(HitSurface {
                Vector3f(
                    LLB[0] + reng.getUniformDouble(0, 1) * (URF[0] - LLB[0]),
                    which ? LLB[1] : URF[1],
                    LLB[2] + reng.getUniformDouble(0, 1) * (URF[2] - LLB[2])
                ),
                Vector3f(0, which ? -1 : 1, 0)
            }, pdf);
        }
    }
};