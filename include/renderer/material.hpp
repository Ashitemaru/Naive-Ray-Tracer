#pragma once

#include "image.hpp"

class Material {
protected:
    Image *texture;

public:
    Material(): texture(nullptr) { }

    virtual ~Material() { delete texture; }

    Vector3f getTexturePixel(const Vector2f &cord) const {
        if (this->texture == nullptr)
            return Vector3f::ZERO;

        double x_cord = cord.x();
        double y_cord = cord.y();
        int x = (x_cord - std::floor(x_cord)) * this->texture->getWidth();
        int y = (y_cord - std::floor(y_cord)) * this->texture->getHeight();

        return this->texture->getPixel(
            x >= this->texture->getWidth() ? this->texture->getWidth() - 1 : x,
            y >= this->texture->getHeight() ? this->texture->getHeight() - 1 : y
        );
    }

    void saveTexture(const char *filename) const {
        this->texture->saveBMP(filename);
    }

    virtual Vector3f shade(const Vector3f &in, const Vector3f &out, bool from_light) const = 0;
};