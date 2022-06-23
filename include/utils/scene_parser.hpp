#pragma once

#include "renderer/camera.hpp"
#include "renderer/light.hpp"
#include "renderer/hit.hpp"
#include "geometry/group.hpp"

class SceneParser {
private:
    FILE *file;
    Camera *camera;

    Vector3f backgroundColor;
    Vector3f ambientColor;

    int numLights;
    Light **lights;

    int numMaterials;
    Material **materials;
    Material *currentMaterial;

    Group *group;

    void parseFile();

    void parsePerspectiveCamera();
    void parseLensCamera();

    void parseBackground();

    void parseLights();
    PointLight *parsePointLight();
    AreaLight *parseAreaLight();
    DirectedPointLight *parseDirectedPointLight();

    void parseMaterials();
    Lambert *parseLambertMaterial();
    Phong *parsePhongMaterial();
    Specular *parseSpecularMaterial();
    Transparent *parseTransparentMaterial();
    Material *parseGenericMaterial();

    Object3D *parseObject(char token[MAX_PARSER_TOKEN_LENGTH]);
    Group *parseGroup();
    Sphere *parseSphere();
    Plane *parsePlane();
    Rectangle * parseRectangle();
    Triangle *parseTriangle();
    Mesh *parseTriangleMesh();
    Transform *parseTransform();

    int getToken(char token[MAX_PARSER_TOKEN_LENGTH]);

    Vector3f readVector3f();

    float readFloat();
    int readInt();

public:
    SceneParser(const char *filename) {
        // TODO
    }

    Camera *getCamera() {
        // TODO
    }

    int getLightNum() {
        // TODO
    }

    Light *getLight(int index) {
        // TODO
    }

    bool intersect(const Ray &r, Hit &h, double tmin, bool &isLight, int &lightId) {
        // TODO
    }
};