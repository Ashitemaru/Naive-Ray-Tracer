#include "utils/octree.h"
#include "geometry/mesh.h"

void Octree::release(OctNode *n) {
    delete n->bbox;
    if (!n->leaf)
        for (int i = 0; i < 8; ++i)
            release(n->child[i]);
    delete n;
}

OctNode *Octree::build(Mesh *mesh, BBox *bbox, const std::vector<int> &ids, int depth) {
    assert(bbox != nullptr);
    if (ids.empty()) {
        delete bbox;
        return nullptr;
    }

    OctNode *now = new OctNode;
    now->bbox = bbox;

    // When the dividing result meets requirements
    if (ids.size() <= MAX_TRI_IN_A_BOX || depth >= MAX_TREE_DEPTH) {
        now->leaf = true;
        now->faceIds = ids;
        return now;
    }

    // Otherwise, we need continue dividing
    std::vector<int> splitIds[8];
    for (int id : ids) {
        const TriangleInfo &info = mesh->triangles[id];
        auto &vList = mesh->vertices;

        for (int i = 0; i < 8; ++i) {
            if (bbox->getChild(i)->triIntersectBox(
                    vList[info.vId[0]], vList[info.vId[1]], vList[info.vId[2]]))
                splitIds[i].push_back(i);
        }
    }

    // Recursive
    for (int i = 0; i < 8; i++)
        now->child[i] = build(mesh, bbox->getChild(i), splitIds[i], depth + 1);
    now->leaf = false;
    return now;
}

bool Octree::traverseIntersect(const Mesh *mesh, OctNode *node, const Ray &r, Hit &h, float tmin) const {
    if (node == nullptr)
        return false;

    if (node->leaf) {
        bool result = false;
        for (int id : node->faceIds) {
            auto &info = mesh->triangles[id];
            Triangle triangle(
                mesh->vertices[info.vId[0]],
                mesh->vertices[info.vId[1]],
                mesh->vertices[info.vId[2]],
                info.material);
            if (info.validNormal)
                triangle.setNormal(
                    mesh->normals[info.nId[0]],
                    mesh->normals[info.nId[1]],
                    mesh->normals[info.nId[2]]);
            if (info.textured)
                triangle.setCord(
                    mesh->cords[info.cordId[0]],
                    mesh->cords[info.cordId[1]],
                    mesh->cords[info.cordId[2]]);
            result |= triangle.intersect(r, h, tmin);
        }
        return result;
    }

    std::vector<std::pair<float, int>> tList;
    for (int octant = 0; octant < 8; octant++) {
        Hit hit = h;
        if (
            node->child[octant] != nullptr &&
            node->child[octant]->bbox->intersect(r, hit, tmin))
            tList.push_back(std::make_pair(hit.t, octant));
    }

    std::sort(tList.begin(), tList.end());
    bool result = false;
    for (auto &p : tList) {
        result |= traverseIntersect(mesh, node->child[p.second], r, h, tmin);
        if (result && node->child[p.second]->bbox->in(h.surface.position))
            break;
    }
    return result;
}

Octree::Octree(Mesh *m, BBox *rootBB) {
    int triangleNum = m->triangles.size();
    std::vector<int> ids;
    for (int i = 0; i < triangleNum; ++i)
        ids.push_back(i);

    root = build(m, rootBB, ids, 0);
}

Octree::~Octree() { release(root); }

bool Octree::intersect(const Mesh *mesh, const Ray &r, Hit &h, float tmin) const {
    Hit htmp = h;
    if (!this->root->bbox->intersect(r, htmp, tmin))
        return false;

    if (!traverseIntersect(mesh, this->root, r, h, tmin))
        return false;
    return true;
}