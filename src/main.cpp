#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Usage: ./bin/NAIVE_RAY_TRACER <input scene file> <output bmp file>" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    return 0;
}