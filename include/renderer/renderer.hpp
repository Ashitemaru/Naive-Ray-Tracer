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
        int lightNum = parser.getLightNum();

#pragma omp parallel for schedule(dynamic, 100)
        // Traverse all the photons
        for (int id = 0; id < this->photonNum; ++id) {
            // Randomly get a light source
            RandomEngine &reng = rengList[omp_get_thread_num()];
            int lightId = reng.getUniformInt(0, lightNum);
            Light *light = parser.getLight(lightId);

            // Sample a ray from this light source
            double pdf;
            Vector3f power;
            Ray ray = light->sampleRay(power, pdf, reng);

            if (pdf < 0) continue; // Invalid ray, pass it
            power = power / std::max(1e-6, pdf) * lightNum;

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

                // TODO: Randomly decay the photon
                if (dep > 0) {
				    double prob = std::max(power[0], std::max(power[1], power[2]));
				    prob = prob > 1. ? 1. : prob;

                    if (reng.getUniformDouble(0, 1) >= prob) break;
                    power = power / prob;
                }

                // Get the next intersection
                Hit hit;
                bool isLight;
                int lightId;
                bool isIntersect = parser.intersect(ray, hit, 1e-6, isLight, lightId);
                if (!isIntersect) break; // No intersect, photon travels straightly

                // Calc the new power of photon & direction of it
                Material *material = hit.getMaterial();
                HitSurface surface = hit.getSurface();

                /**
                 * @ref: https://github.com/Numendacil/Graphics/blob/master/include/render.hpp
                 */

                // The input ray
                /*
                Vector3f in = -ray.d.normalized();
                double pdf;
                RefType type;
                Vector3f out;
                Vector3f tangent = GetPerpendicular(surface.normal);
                Vector3f binormal = Vector3f::cross(surface.normal, tangent).normalized();
                Vector3f co = material->SampleOutDir(AbsToRel(tangent, binormal, surface.normal, in), out, TransportMode::LIGHT, pdf, type, rng);
                if (type == RefType::DIFFUSE)
                {
                    #pragma omp critical
                    {
                        Photons.push_back(Photon{surface.position, in, power});
                    }
                }
                if (surface.HasTexture && material->HasTexture())
                {
                    co = co * material->GetTexture(surface.texcoord);
                }
                out = RelToAbs(tangent, binormal, surface.normal, out);
                ray = Ray(surface.position, out);
                power = power * co / std::max(pdf, 1e-6)  
                    * std::abs(Vector3f::dot(out, surface.geonormal)) * std::abs(Vector3f::dot(in, surface.normal)) / std::abs(Vector3f::dot(in, surface.geonormal));
                    */
            }
        }
    }

    Vector3f getRadiance(const Ray &r, SceneParser &parser, RandomEngine &reng) {

    }

public:
    SPPMRenderer() { }
    
    void render(SceneParser &parser, Image &image) {
        std::vector<Vector3f> img(image.getHeight() * image.getWidth());

        // Initialize random engines
        std::vector<RandomEngine> rengList(omp_get_max_threads());
        for (int i = 0; i < (int) rengList.size(); i++) {
            rengList[i].setSeed(rengList[i].getUniformInt(0, rengList.size()) + i * rengList.size());
        }

        for (int i = 0; i < this->iter; i++) {
            std::cout << "Now at iteration: " << i << std::endl;

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
                    Vector3f colorTmp = img[j + i * image.getHeight()] / (i + 1);

                    double maxColor = 1.;
                    for (int k = 0; k < 3; k++) {
                        colorTmp[k] = std::pow(colorTmp[k], 1.0f / parser.getCamera()->getGamma());
                        maxColor = std::max(maxColor, colorTmp[k]);
                    }

                    renderImg.setPixel(i, j, colorTmp / maxColor);

#pragma omp critical
                    {
                        std::cout << "i = " << i << ", j = " << j << " finished." << std::endl;
                    }
                }
            }

            // Save the temporary result & step the search radius
            renderImg.saveBMP(("tmp/" + std::to_string(i) + ".bmp").c_str());
            this->searchRadius *= sqrt((i + this->alpha) / (i + 1));
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