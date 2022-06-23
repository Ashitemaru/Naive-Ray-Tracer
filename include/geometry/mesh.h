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

struct TriangleInfo {
    int vId[3];
    int nId[3];
    int cordId[3];

    bool validNormal;
    bool textured;

    Material *material;
};

class Octree;

class Mesh : public Object3D {
public:
    std::vector<Vector3f> vertices;
    std::vector<Vector3f> normals;
    std::vector<Vector2f> cords;

    std::vector<TriangleInfo> triangles;
    std::map<std::string, Material *> materialMap;

    Octree *tree;

public:
    Mesh(const char *filename, Material *material);

    virtual ~Mesh();

    void parseMTL(const char *filename);

    virtual bool intersect(const Ray &r, Hit &h, double tmin) const;

    virtual std::pair<HitSurface, double> samplePoint(RandomEngine &reng) const;
};