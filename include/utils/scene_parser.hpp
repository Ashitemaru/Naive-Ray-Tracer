#pragma once

#include "renderer/camera.hpp"
#include "renderer/light.hpp"
#include "renderer/hit.hpp"
#include "geometry/group.hpp"
#include "geometry/mesh.h"
#include "geometry/bbox.hpp"
#include "geometry/object3d.hpp"
#include "geometry/plane.hpp"
#include "geometry/rectangle.hpp"
#include "geometry/sphere.hpp"
#include "geometry/transform.hpp"
#include "geometry/triangle.hpp"

#define MAX_PARSER_TOKEN_LENGTH 1024

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
    double readDouble();
    int readInt();

public:
    SceneParser() = delete;
	SceneParser(const char *filename);

	~SceneParser();

    Camera *getCamera() const {
		return camera;
	}

	Vector3f getBackgroundColor() const {
		return backgroundColor;
	}

	Vector3f getAmbient() const {
		return ambientColor;
	}

	int getNumLights() const {
		return numLights;
	}

	Light *getLight(int i) const {
		assert(i >= 0 && i < numLights);
		return lights[i];
	}

	int getNumMaterials() const {
		return numMaterials;
	}

	Material *getMaterial(int i) const {
		assert(i >= 0 && i < numMaterials);
		return materials[i];
	}

	Group *getGroup() const {
		return group;
	}

	bool intersect(const Ray &r, Hit &h, double tmin, bool& isLight, int& LightIdx) const {
		bool objIntersect = group->intersect(r, h, tmin);
		isLight = false;
		for (int i = 0; i < numLights; i++) {
			isLight |= lights[i]->intersect(r, h, tmin);
			if (isLight) LightIdx = i;
		}
		return isLight | objIntersect;
	}
};