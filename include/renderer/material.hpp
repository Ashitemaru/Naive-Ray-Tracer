#pragma once

#include "utils/trans.hpp"
#include "utils/image.hpp"
#include "utils/random_engine.hpp"

struct IntersectResult {
    Vector3f x;
    Vector3f out;
    double pdf;
    bool isDiffuse;
};

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

    /**
     * @note: All use relative coordination.
     */
    virtual Vector3f shade(const Vector3f &in, const Vector3f &out, bool fromLight) const = 0;
    virtual IntersectResult getOutputRay(const Vector3f &in, bool fromLight, RandomEngine &reng) const = 0;
};

class Specular : public Material {
protected:
    Vector3f color;

public:
    Specular(const Vector3f &c)
        : color(c) { }

    Specular(const Vector3f &c, const char *filename) {
        this->color = c;
        this->texture = Image::loadBMP(filename);
    }

    virtual Vector3f shade(const Vector3f &in, const Vector3f &out, bool fromLight) const override {
        return Vector3f::ZERO;
    }

    virtual IntersectResult getOutputRay(const Vector3f &in, bool fromLight, RandomEngine &reng) const override {
        Vector3f out = Trans::reflect(in, Vector3f(0, 0, 1));
        return IntersectResult {
            .x = this->color / (std::abs(out[2]) + 1e-6),
            .out = out,
            .pdf = 1,
            .isDiffuse = false,
        };
    }
};

class Transparent : public Material {
protected:
    double n;
    Vector3f color;

public:
    Transparent(const Vector3f &c, double _n)
        : color(c), n(_n) { }

    Transparent(const Vector3f &c, double _n, const char *filename) {
        this->color = c;
        this->n = _n;
        this->texture = Image::loadBMP(filename);
    }

    virtual Vector3f shade(const Vector3f &in, const Vector3f &out, bool fromLight) const override {
        return Vector3f::ZERO;
    }

    virtual IntersectResult getOutputRay(const Vector3f &in, bool fromLight, RandomEngine &reng) const override {
        Vector3f reflectOut = Trans::reflect(in, Vector3f(0, 0, 1));
		Vector3f refractOut = in[2] >= 0 // Going into the medium
            ? Trans::refract(in, Vector3f(0, 0, 1), 1., n)
            : Trans::refract(in, Vector3f(0, 0, -1), n, 1.);
		double scale = in[2] >= 0 // Going into the medium
            ? 1. / (n * n)
            : n * n;
		
		double coi = abs(reflectOut[2]), cot = abs(refractOut[2]);
		double rs = (coi - n * cot) * (coi - n * cot) / ((coi + n * cot) * (coi + n * cot));
		double rp = (cot - n * coi) * (cot - n * coi) / ((cot + n * coi) * (cot + n * coi));

		if (reng.getUniformDouble(0, 1) < (rs + rp) / 2.) // Reflect
            return IntersectResult {
                .x = this->color / std::max((double) coi, 1e-6),
                .out = reflectOut,
                .pdf = 1.,
                .isDiffuse = false,
            };
		else
            return IntersectResult {
                .x = fromLight
                    ? this->color / std::max((double) cot, 1e-6)
                    : scale * this->color / std::max((double) cot, 1e-6),
                .out = refractOut,
                .pdf = 1.,
                .isDiffuse = false,
            };
    }
};

class Phong : public Material {
private:
    Vector3f diffuseColor;
    Vector3f specularColor;
    double shininess;

public:
    Phong(const Vector3f &d, const Vector3f &s, double _shininess)
        : diffuseColor(d), specularColor(s), shininess(_shininess) { }

    Phong(const Vector3f &d, const Vector3f &s, double _shininess, const char *filename) {
        this->diffuseColor = d;
        this->specularColor = s;
        this->shininess = _shininess;
        this->texture = Image::loadBMP(filename);
    }

    virtual Vector3f shade(const Vector3f &in, const Vector3f &out, bool fromLight) const override {
        if (in[2] * out[2] < 0)
            return Vector3f::ZERO;

        double cos_ = Vector3f::dot(out, Trans::reflect(in, Vector3f(0, 0, 1)));
		cos_ = (cos_ > 0) ? cos_ : 0;

		// Modified Phong model
		return diffuseColor / M_PI
            + specularColor * pow(cos_, shininess) * (2 + shininess) / (2 * M_PI);
    }

    virtual IntersectResult getOutputRay(const Vector3f &in, bool fromLight, RandomEngine &reng) const override {
        Vector3f total = this->diffuseColor + this->specularColor;

		double probR = std::max(total[0], std::max(total[1], total[2]));
		probR = (probR > 1.) ? 1. : probR;
		double probD =
            probR * (diffuseColor[0] + diffuseColor[1] + diffuseColor[2]) / (total[0] + total[1] + total[2]);
    
		double posix = reng.getUniformDouble(0, 1);

		if (posix < probD) { // Diffuse
			double phi = 2 * M_PI * reng.getUniformDouble(0, 1);
			double t = std::sqrt(reng.getUniformDouble(0, 1));

            return IntersectResult {
                .x = this->diffuseColor / M_PI,
                .out = Vector3f(std::sqrt(1 - t * t) * std::cos(phi), std::sqrt(1 - t * t) * std::sin(phi), t),
                .pdf = t * probR / M_PI,
                .isDiffuse = true,
            };
		} else if (posix < probR) { // Specular
			Vector3f x = Trans::reflect(in, Vector3f(0, 0, 1));
			Vector3f y = Trans::generateVertical(x);
			Vector3f z = Vector3f::cross(x, y).normalized();

			double phi = 2 * M_PI * reng.getUniformDouble(0, 1);
			double t = std::pow(std::sqrt(reng.getUniformDouble(0, 1)), 1. / (1. + this->shininess));
            Vector3f out = Trans::localToWorld(
                y, z, x,
                Vector3f(
                    std::sqrt(1 - t * t) * std::cos(phi),
                    std::sqrt(1 - t * t) * std::sin(phi),
                    t
                )
            );

            return IntersectResult {
                .x = shade(in, out, fromLight) - this->diffuseColor / M_PI,
                .out = out,
                .pdf = probR * (shininess + 2.) * std::pow(t, shininess) / (2. * M_PI),
                .isDiffuse = true,
            };
		} else { // Absorbed
            return IntersectResult {
                .x = Vector3f::ZERO,
                .out = Vector3f::ZERO,
                .pdf = 1.,
                .isDiffuse = true,
            };
		}
    }
};

class Lambert : public Material {
private:
    Vector3f color;

public:
    Lambert(const Vector3f &c)
        : color(c) { }

    Lambert(const Vector3f &c, const char *filename) {
        this->color = c;
        this->texture = Image::loadBMP(filename);
    }

    virtual Vector3f shade(const Vector3f &in, const Vector3f &out, bool fromLight) const override {
        if (in[2] * out[2] < 0)
			return Vector3f::ZERO;
		return this->color / M_PI;
    }

    virtual IntersectResult getOutputRay(const Vector3f &in, bool fromLight, RandomEngine &reng) const override {
        double phi = 2 * M_PI * reng.getUniformDouble(0, 1);
        double t = std::sqrt(reng.getUniformDouble(0, 1));

        Vector3f out = Vector3f(std::sqrt(1 - t * t) * std::cos(phi), std::sqrt(1 - t * t) * std::sin(phi), t);

        return IntersectResult {
            .x = shade(in, out, fromLight),
            .out = out,
            .pdf = t / M_PI,
            .isDiffuse = true,
        };
    }
};

class General : public Material { // The general model in .mtl file
    /**
     * @ref: https://github.com/Numendacil/Graphics/blob/master/include/material.hpp
     */

    // TODO

};
