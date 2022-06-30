#pragma once

#include "photon/photon_map.hpp"
#include "utils/scene_parser.hpp"
#include "utils/image.hpp"
#include "utils/random_engine.hpp"
#include "renderer/ray.hpp"
#include "renderer/hit.hpp"

#include <vector>
#include <omp.h>
#include <iostream>
#include <cmath>

class SPPMRenderer {
private:
    PhotonMap gMap;

    int photonNum;
    int rayNum;

    int iter;
    int depth;

    double searchRadius;
    double alpha;

    void buildPhotonMap(SceneParser &parser, std::vector<RandomEngine> &rengList) {
        std::vector<Photon> photonList;
        int lightNum = parser.getNumLights();

#pragma omp parallel for schedule(dynamic, 100)
        // Traverse all the photons
        for (int id = 0; id < this->photonNum; ++id) {
            // Randomly get a light source
            RandomEngine &reng = rengList[omp_get_thread_num()];
            int lightId = reng.getUniformInt(0, lightNum - 1);
            Light *light = parser.getLight(lightId);

            // Sample a ray from this light source
            auto result = light->sampleRay(reng);
            Ray ray = result.ray;
            Vector3f power = result.power;

            if (result.pdf < 0) continue; // Invalid ray, pass it
            power = power / std::max(1e-6, result.pdf) * lightNum;

            auto validVector = [&](const Vector3f &v) -> bool {
                return !(
                    v[0] < 0 || std::isinf(v[0]) || std::isnan(v[0]) ||
                    v[1] < 0 || std::isinf(v[1]) || std::isnan(v[1]) ||
                    v[2] < 0 || std::isinf(v[2]) || std::isnan(v[2])
                );
            };

            // Let the photon travel & bump on objects, calc its power
            for (int dep = 0; dep < this->depth; ++dep) {
                if (!validVector(power)) break; // Invalid photon, pass it

                // Get the next intersection
                Hit hit;
                bool isLight;
                int lightId;
                bool isIntersect = parser.intersect(ray, hit, 1e-6, isLight, lightId);
                if (!isIntersect) break; // No intersect, photon travels straightly

                // Calc the new power of photon & direction of it
                Material *material = hit.material;
                HitSurface surface = hit.surface;

                /**
                 * @ref: https://github.com/Numendacil/Graphics/blob/master/include/render.hpp
                 */

                // The input ray
                Vector3f in = -ray.d.normalized();
                Vector3f x = surface.normal;
                Vector3f y = Trans::generateVertical(x);
                Vector3f z = Vector3f::cross(x, y).normalized();
                auto res = material->getOutputRay(Trans::worldToLocal(y, z, x, in), true, reng);
                Vector3f co = res.x;

                if (res.isDiffuse) {
#pragma omp critical
                    {
                        photonList.push_back(Photon { surface.position, in, power });
                    }
                }
                if (surface.hasTexture && material->textured())
                    co = co * material->getTexturePixel(surface.cord);

                Vector3f out = Trans::localToWorld(y, z, x, res.out);
                ray = Ray(surface.position, out);
                power =
                    power * co / std::max(res.pdf, 1e-6) *
                    std::abs(Vector3f::dot(out, surface.geoNormal)) *
                    std::abs(Vector3f::dot(in, x)) /
                    std::abs(Vector3f::dot(in, surface.geoNormal));
            }
        }
        gMap.set(photonList);
        gMap.constructTree();
    }

    Vector3f getRadiance(const Ray &r, SceneParser &parser, RandomEngine &reng) {
        Ray ray = r;
        Vector3f power(1, 1, 1);

        for (int depth = 0; depth < this->depth; depth++) {
            Hit hit;
            bool isLight;
            int lightId = 0;
            if (!parser.intersect(ray, hit, 1e-6, isLight, lightId))
                return parser.getBackgroundColor();

            Vector3f dir = ray.d.normalized();

            Material* material = hit.material;
            HitSurface surface = hit.surface;

            Vector3f x = surface.normal;
            Vector3f y = Trans::generateVertical(x);
            Vector3f z = Vector3f::cross(x, y).normalized();
            auto res = material->getOutputRay(Trans::worldToLocal(y, z, x, -dir), false, reng);

            if (res.isDiffuse) {
                if (isLight)
                    return power * (getPhotonRadiance(dir, hit, parser, reng) 
                        + parser.getLight(lightId)->getIllumin(dir) * std::abs(Vector3f::dot(dir, x)));
                return power * getPhotonRadiance(dir, hit, parser, reng);
            }

            if (surface.hasTexture && material->textured())
                power = power * material->getTexturePixel(surface.cord);

            Vector3f out = Trans::localToWorld(y, z, x, res.out);
            ray = Ray(surface.position, out);
            power = power * res.x * std::abs(Vector3f::dot(out, x)) / std::max(res.pdf, 1e-6);
            if (power.length() < 1e-5) break;
        }
        return power;
    }

    Vector3f getPhotonRadiance(const Vector3f& v, const Hit& hit, SceneParser& parser, RandomEngine& reng) {
        const HitSurface& surface = hit.surface;
        Material* material = hit.material;

        std::vector<Photon *> res = gMap.IRSearch(surface.position, searchRadius * searchRadius);
        Vector3f x = surface.normal;
        Vector3f y = Trans::generateVertical(x);
        Vector3f z = Vector3f::cross(x, y).normalized();
        Vector3f in = Trans::worldToLocal(y, z, x, -v);

        Vector3f color = Vector3f::ZERO;
        for (auto ph_ptr : res) {
            color +=
                ph_ptr->power * material->shade(
                    in,
                    Trans::worldToLocal(y, z, x, ph_ptr->direction),
                    false
                );
        }
        if (surface.hasTexture && hit.material->textured())
            color = color * hit.material->getTexturePixel(surface.cord);

        return (
            color / (M_PI * searchRadius * searchRadius * photonNum) +
            parser.getAmbient() * material->shade(in, Vector3f(0, 0, 1), false)
        );
    }

public:
    SPPMRenderer(int n, int i, int d, int nrays, double r, double a)
        : photonNum(n), iter(i), depth(d), rayNum(nrays), searchRadius(r), alpha(a) { }
    
    void render(SceneParser &parser, Image &image) {
        std::vector<Vector3f> img(image.getHeight() * image.getWidth());

        // Initialize random engines
        std::vector<RandomEngine> rengList(omp_get_max_threads());
        for (int i = 0; i < (int) rengList.size(); i++) {
            rengList[i].setSeed(rengList[i].getUniformInt(0, rengList.size() - 1) + i * rengList.size());
        }

        for (int iter_ = 0; iter_ < this->iter; iter_++) {
            std::cout << "Now at iteration: " << iter_ << std::endl;

            this->buildPhotonMap(parser, rengList);
            std::cout << "Finish building Photon Map" << std::endl;

            Image renderImg(image.getWidth(), image.getHeight()); 

#pragma omp parallel for collapse(2) schedule(dynamic, 5)
            // Traverse all the pixels
            for (int i = 0; i < image.getWidth(); i++) {
                for (int j = 0; j < image.getHeight(); j++) {
                    RandomEngine& reng = rengList[omp_get_thread_num()];
                    Vector3f color = Vector3f::ZERO;

                    // Sample rays
                    auto validVector = [&](const Vector3f &v) -> bool {
                        return !(
                            v[0] < 0 || std::isinf(v[0]) || std::isnan(v[0]) ||
                            v[1] < 0 || std::isinf(v[1]) || std::isnan(v[1]) ||
                            v[2] < 0 || std::isinf(v[2]) || std::isnan(v[2])
                        );
                    };

                    for (int k = 0; k < this->rayNum; k++) {
                        Ray camRay = parser.getCamera()->sampleRay(i, j, reng);
                        Vector3f x = this->getRadiance(camRay, parser, reng);
                        
                        if (!validVector(x)) continue; // When radiance is invalid, pass it
                        color += x;
                    }

                    // Save the color into the result image
                    img[j + i * image.getHeight()] += color / this->rayNum;

                    // Save this pass
                    Vector3f colorTmp = img[j + i * image.getHeight()] / (iter_ + 1);

                    double maxColor = 1.;
                    for (int k = 0; k < 3; k++) {
                        colorTmp[k] = std::pow(colorTmp[k], 1. / parser.getCamera()->getGamma());
                        maxColor = std::max(maxColor, colorTmp[k]);
                    }

                    renderImg.setPixel(i, j, colorTmp / maxColor);
                }
            }

            // Save the temporary result & step the search radius
            renderImg.saveBMP(("tmp/" + std::to_string(iter_) + ".test.bmp").c_str());
            searchRadius *= sqrt((iter_ + this->alpha) / (iter_ + 1));
        }

        // Pass out the render result
        for (int i = 0; i < image.getWidth(); i++)
            for (int j = 0; j < image.getHeight(); j++) {
                Vector3f color = img[j + i * image.getHeight()] / this->iter;
                double maxColor = 1.;

                // TODO: Understand operations here
                for (int k = 0; k < 3; k++) {
                    color[k] = std::pow(color[k], 1.0f / parser.getCamera()->getGamma());
                    maxColor = std::max(maxColor, color[k]);
                }
                image.setPixel(i, j, color / maxColor);
            }
    }
};