#include "geometry/mesh.h"
#include "utils/octree.h"

Mesh::Mesh(const char *filename, Material *material) {
    tree = nullptr;

    std::ifstream f;
    f.open(filename);
    if (!f.is_open())
        return;

    std::string line;
    std::string tok;
    Vector3f max(-INFINITY, -INFINITY, -INFINITY);
    Vector3f min(INFINITY, INFINITY, INFINITY);

    Material *curMaterial = this->material;
    while (true) {
        std::getline(f, line);
        if (f.eof())
            break;

        if (line.size() < 3)
            continue;
        if (line.at(0) == '#')
            continue;

        std::stringstream ss(line);
        ss >> tok;
        if (tok == "mtllib") {
            std::string mtlfilename;
            ss >> mtlfilename;
            parseMTL(mtlfilename.c_str());
        } else if (tok == "usemtl") {
            std::string name;
            ss >> name;
            if (materialMap.count(name))
                curMaterial = materialMap[name];
            else
                curMaterial = this->material;
        } else if (tok == "v") {
            Vector3f vec;
            ss >> vec[0] >> vec[1] >> vec[2];
            for (int i = 0; i < 3; i++)
            {
                max[i] = std::max(max[i], vec[i]);
                min[i] = std::min(min[i], vec[i]);
            }
            vertices.push_back(vec);
        } else if (tok == "vt") {
            Vector2f cord;
            ss >> cord[0];
            ss >> cord[1];
            cords.push_back(cord);
        } else if (tok == "vn") {
            Vector3f norm;
            ss >> norm[0] >> norm[1] >> norm[2];
            normals.push_back(norm);
        } else if (tok == "f") {
            std::string token[3];
            TriangleInfo info;
            ss >> token[0] >> token[1] >> token[2];
            for (int i = 0; i < 3; i++) {
                std::istringstream iss(token[i]);
                std::string item;
                std::getline(iss, item, '/'); // Vertex
                info.vId[i] = std::stoi(item) - 1;

                if (std::getline(iss, item, '/')) { // Texture
                    if (!item.empty()) {
                        info.cordId[i] = std::stoi(item) - 1;
                        info.textured = true;
                    }
                }

                if (std::getline(iss, item, '/')) { // Normal
                    info.nId[i] = std::stoi(item) - 1;
                    info.validNormal = true;
                }
            }
            info.material = curMaterial;
            triangles.push_back(info);
        }
    }

    f.close();

    BBox *bbox = new BBox(max, min);
    this->tree = new Octree(this, bbox);
}

Mesh::~Mesh() {
    if (tree)
        delete tree;
}

void Mesh::parseMTL(const char *filename) {
    std::ifstream f;
    f.open(filename);
    if (!f.is_open())
        return;

    std::string line, tok;
    bool isReading = false;
    std::string mtlName, texPath;
    Vector3f Ka, Kd, Ks;
    float Ns, Ni, d;
    int illum;
    bool textured;

    auto generate = [&](
                        const Vector3f &Ka,
                        const Vector3f &Kd,
                        const Vector3f &Ks,
                        float Ns,
                        float Ni,
                        float d,
                        int illum,
                        bool hasTexture,
                        const char *filename) -> Material *
    {
        if (illum == 0 || illum == 1)
            return hasTexture
                       ? new Lambert(Kd, filename)
                       : new Lambert(Kd);
        else if (illum == 2)
            return hasTexture
                       ? new Phong(Kd, Ks, Ns, filename)
                       : new Phong(Kd, Ks, Ns);
        else if (illum == 5)
            return hasTexture
                       ? new Specular(Ks, filename)
                       : new Specular(Ks);
        else if (illum == 7)
            return hasTexture
                       ? new Transparent(Ks, Ni, filename)
                       : new Transparent(Ks, Ni);
        else
            return hasTexture
                       ? new General(Ka, Kd, Ks, Ns, Ni, d, filename)
                       : new General(Ka, Kd, Ks, Ns, Ni, d);
    };

    while (true) {
        std::getline(f, line);
        if (f.eof()) {
            if (isReading)
                materialMap[mtlName] = generate(Ka, Kd, Ks, Ns, Ni, d, illum, textured, texPath.c_str());
            break;
        }
        if (line.empty()) {
            if (isReading)
                materialMap[mtlName] = generate(Ka, Kd, Ks, Ns, Ni, d, illum, textured, texPath.c_str());
            isReading = false;
            continue;
        }
        if (line.at(0) == '#')
            continue;

        std::stringstream ss(line);
        ss >> tok;
        if (tok == "newmtl") {
            isReading = true;
            Ka = Vector3f(1, 1, 1);
            Kd = Vector3f(1, 1, 1);
            Ks = Vector3f(1, 1, 1);
            Ns = 0.;
            Ni = 1.;
            d = 1.;
            illum = -1;
            textured = false;
            ss >> mtlName;
        } else if (tok == "Ka") {
            ss >> Ka[0] >> Ka[1] >> Ka[2];
        } else if (tok == "Kd") {
            ss >> Kd[0] >> Kd[1] >> Kd[2];
        } else if (tok == "Ks") {
            ss >> Ks[0] >> Ks[1] >> Ks[2];
        } else if (tok == "Ns") {
            ss >> Ns;
        } else if (tok == "Ni") {
            ss >> Ni;
        } else if (tok == "d") {
            ss >> d;
        } else if (tok == "illum") {
            ss >> illum;
        } else if (tok.substr(0, 3) == "map") {
            ss >> texPath;
            textured = true;
        }
    }
}

bool Mesh::intersect(const Ray &r, Hit &h, double tmin) const {
    return tree->intersect(this, r, h, tmin);
}

std::pair<HitSurface, double> Mesh::samplePoint(RandomEngine &reng) const {
    int triangleNum = triangles.size();
    int id = reng.getUniformInt(0, triangleNum - 1);

    auto info = triangles[id];
    Triangle triangle(vertices[info.vId[0]], vertices[info.vId[1]], vertices[info.vId[2]], info.material);
    if (info.validNormal)
        triangle.setNormal(normals[info.nId[0]], normals[info.nId[1]], normals[info.nId[2]]);
    if (info.textured)
        triangle.setCord(cords[info.cordId[0]], cords[info.cordId[1]], cords[info.cordId[2]]);

    auto pair = triangle.samplePoint(reng);
    return std::make_pair(pair.first, 1. / triangleNum * pair.second);
}