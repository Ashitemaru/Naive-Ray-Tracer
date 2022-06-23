#pragma once

#include "renderer/camera.hpp"
#include "renderer/light.hpp"
#include "renderer/hit.hpp"

class SceneParser {
public:
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