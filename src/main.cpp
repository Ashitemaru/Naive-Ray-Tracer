#include <iostream>
#include <string>

#include "utils/scene_parser.hpp"
#include "utils/image.hpp"
#include "renderer/renderer.hpp"
#include "renderer/camera.hpp"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Usage: ./bin/NAIVE_RAY_TRACER <input scene file> <output bmp file>" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];

    SceneParser parser(inputFile.c_str());
    Camera *camera = parser.getCamera();
    Image img(camera->getWidth(), camera->getHeight());
    SPPMRenderer renderer(400000, 400, 100, 16, 0.5, 0.75);

    renderer.render(parser, img);
    img.saveBMP(outputFile.c_str());

    return 0;
}