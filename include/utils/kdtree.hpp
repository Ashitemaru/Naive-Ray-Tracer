#pragma once

#include <functional>
#include <algorithm>
#include <omp.h>
#include <vecmath.h>
#include <vector>
#include <queue>

#include "photon/photon.hpp"

/**
 * @note: Actually it is a 3D tree.
 */
struct PhotonNode {
    Photon *photon;
    int direction; // 0 - x, 1 - y, 2 - z

    PhotonNode *left;
    PhotonNode *right;
};

class KDTree {
private:
    PhotonNode *root;

    PhotonNode *build(Photon *base, int len, int depth) {
        if (len <= 0) return nullptr;

        // Resort the data array
        int direction = depth % 3;
        std::sort(base, base + len, [&](Photon &a, Photon &b) {
            return a.pos[direction] < b.pos[direction];
        });

        // Split at the middle
        int m = (len - 1) >> 1;
        PhotonNode *now = new PhotonNode;
        now->direction = direction;
        now->photon = &base[m];

        const int maxParaLevel = (int) std::log2(omp_get_max_threads());
        if (depth < maxParaLevel + 1) {
#pragma omp task firstprivate(now)
            now->left = this->build(base, m, depth + 1);
#pragma omp task firstprivate(now)
            now->right = this->build(base + m + 1, len - m - 1, depth + 1);
#pragma omp taskwait
        } else {
            now->left = this->build(base, m, depth + 1);
            now->right = this->build(base + m + 1, len - m - 1, depth + 1);
        }

        return now;
    }

    void searchInRange(
        PhotonNode *now,
        std::vector<Photon *> &result,
        const Vector3f &target,
        double d_sq
    ) {
        if (now == nullptr) return;

        double dn_sq = (target - now->photon->pos).squaredLength();
        if (dn_sq < d_sq)
            result.push_back(now->photon);

        double directionDiff = target[now->direction] - now->photon->pos[now->direction];
        searchInRange(
            directionDiff < 0 ? now->left : now->right,
            result, target, d_sq
        );

        if (d_sq > directionDiff * directionDiff)
            searchInRange(
                directionDiff > 0 ? now->left : now->right,
                result, target, d_sq
            );
    }

    void release(PhotonNode *node) {
        if (node == nullptr) return;
        release(node->left);
        release(node->right);
        delete node;
    }

public:
    KDTree(Photon *photonList, int len) {
#pragma omp parallel
{
#pragma omp single
{
        this->root = this->build(photonList, len, 0);
}
}
    }

    ~KDTree() {
        release(this->root);
        this->root = nullptr;
    }

    std::vector<Photon *> IRSearch(const Vector3f &target, double d_sq) {
        std::vector<Photon *> result;
        this->searchInRange(this->root, result, target, d_sq);
        return result;
    }
};