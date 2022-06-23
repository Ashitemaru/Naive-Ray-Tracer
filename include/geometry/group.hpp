#pragma once

#include <vector>

#include "geometry/object3d.hpp"

class Group : public Object3D {
private:
    std::vector<Object3D *> objList;

public:
    Group() = default;

    Group(int size) {
        objList.reserve(size);
    }

    virtual ~Group() override {
        for (Object3D *ptr : objList)
            delete ptr;
    }

    virtual bool intersect(const Ray &r, Hit &h, double tmin) const override {
        bool result = false;
        for (Object3D *item : this->objList)
            result |= item->intersect(r, h, tmin);

        return result;
    }

    virtual std::pair<HitSurface, double> samplePoint(RandomEngine &reng) const override {
        int size = objList.size();
        double pdf = 1. / size;

        int idx = reng.getUniformInt(0, size - 1);
        auto res = objList[idx]->samplePoint(reng);

        pdf *= res.second;
        return std::make_pair(res.first, pdf);
    }

    void append(Object3D *obj) {
        objList.push_back(obj);
    }

    int getSize() {
        return objList.size();
    }
};