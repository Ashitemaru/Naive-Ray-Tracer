#pragma once

#include "geometry/object3d.hpp"
#include "renderer/material.hpp"
#include "geometry/triangle.hpp"
#include "geometry/bbox.hpp"

#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>

#define MAX_TRI_IN_A_BOX 16
#define MAX_TREE_DEPTH 8

struct TriangleInfo;
class Mesh;

struct OctNode {
    BBox *bbox;
    std::vector<int> faceIds;

    bool leaf;
    OctNode *child[8];
};

class Octree {
private:
    OctNode *root;

    void release(OctNode *n);

    OctNode *build(Mesh *mesh, BBox *bbox, const std::vector<int> &ids, int depth);

    bool traverseIntersect(const Mesh *mesh, OctNode* node, const Ray &r, Hit &h, float tmin) const;
public:
    Octree() = delete;

    Octree(Mesh *m, BBox *rootBB);

    ~Octree();

    bool intersect(const Mesh *mesh, const Ray &r, Hit &h, float tmin) const;
};