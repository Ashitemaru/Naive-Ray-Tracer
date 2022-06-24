#pragma once

#include <vector>

#include "utils/kdtree.hpp"

class PhotonMap {
private:
    KDTree *tree;
    std::vector<Photon> photonList;

public:
    PhotonMap() {
        this->tree = nullptr;
    }

    ~PhotonMap() {
        if (this->tree) delete this->tree;
    }

    void set(const std::vector<Photon> &m) {
        this->photonList = m;
    }

    void append(const Photon &p) {
        this->photonList.push_back(p);
    }

    int size() {
        return this->photonList.size();
    }

    Photon &operator[](int index) {
        return this->photonList[index];
    }

    void constructTree() {
        if (this->tree) delete this->tree;
        this->tree = new KDTree(photonList.data(), photonList.size());
    }

    std::vector<Photon *> IRSearch(const Vector3f &target, double d_sq) {
        return this->tree->IRSearch(target, d_sq);
    }
};