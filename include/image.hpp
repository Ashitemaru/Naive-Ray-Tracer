#pragma once

#include <vecmath.h>
#include <cassert>
#include <cstring>

struct BMPHeader {
    char bfType[3];       /* "BM" */
    int bfSize;           /* Size of file in bytes */
    int bfReserved;       /* set to 0 */
    int bfOffBits;        /* Byte offset to actual bitmap data (= 54) */
    int biSize;           /* Size of BITMAPINFOHEADER, in bytes (= 40) */
    int biWidth;          /* Width of image, in pixels */
    int biHeight;         /* Height of images, in pixels */
    short biPlanes;       /* Number of planes in target device (set to 1) */
    short biBitCount;     /* Bits per pixel (24 in this case) */
    int biCompression;    /* Type of compression (0 if no compression) */
    int biSizeImage;      /* Image size, in bytes (0 if no compression) */
    int biXPelsPerMeter;  /* Resolution in pixels/meter of display device */
    int biYPelsPerMeter;  /* Resolution in pixels/meter of display device */
    int biClrUsed;        /* Number of colors in the color table (if 0, use maximum allowed by biBitCount) */
    int biClrImportant;   /* Number of important colors. If 0, all colors are important */
};

class Image {
private:
    int width, height;
    Vector3f *data;

public:
    Image(int _width, int _height) {
        this->width = _width;
        this->height = _height;
        this->data = new Vector3f[_width * _height];
    }

    ~Image() { delete[] data; }

    int height() { return this->height; }

    int width() { return this->width; }

    const Vector3f &getPixel(int x, int y) const {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);
        return data[y * width + x];
    }

    void setPixel(int x, int y, const Vector3f &color) {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);
        data[y * width + x] = color;
    }

    static Image *loadPPM(const char *filename) {
        assert(filename != NULL);

        // File must end in .ppm
        const char *ext = &filename[strlen(filename) - 4];
        assert(!strcmp(ext, ".ppm"));

        FILE *file = fopen(filename, "rb");

        // Misc header information
        int width = 0;
        int height = 0;  
        char tmp[100];
        fgets(tmp, 100, file); 
        assert(strstr(tmp, "P6"));
        fgets(tmp, 100, file); 
        assert(tmp[0] == '#');
        fgets(tmp, 100, file); 
        sscanf(tmp, "%d %d", &width, &height);
        fgets(tmp, 100, file); 
        assert(strstr(tmp, "255"));

        // Data
        Image *answer = new Image(width, height);

        // Flip y so that (0, 0) is bottom left corner
        for (int y = height - 1; y >= 0; y--)
            for (int x = 0; x < width; x++) {
                unsigned char r, g, b;
                r = fgetc(file);
                g = fgetc(file);
                b = fgetc(file);
                Vector3f color(r / 255.0, g / 255.0, b / 255.0);
                answer->setPixel(x, y, color);
            }
        fclose(file);
        return answer;
    }

    void savePPM(const char *filename) const {
        assert(filename != NULL);

        // Must end in .ppm
        const char *ext = &filename[strlen(filename) - 4];
        assert(!strcmp(ext,".ppm"));

        FILE *file = fopen(filename, "w");

        // Misc header information
        assert(file != NULL);
        fprintf(file, "P6\n");
        fprintf(file, "# Creator: Image::SavePPM()\n");
        fprintf(file, "%d %d\n", width, height);
        fprintf(file, "255\n");

        // Data
        // Flip y so that (0, 0) is bottom left corner
        auto clampColor = [&](double c) -> unsigned char {
            int t = int(c * 255);

            if (t < 0) return (unsigned char) 0;
            else if (t > 255) return (unsigned char) 255;
            else return (unsigned char) t;
        };

        for (int y = height - 1; y >= 0; y--)
            for (int x = 0; x < width; x++) {
                Vector3f v = getPixel(x, y);
                fputc(clampColor(v[0]), file);
                fputc(clampColor(v[1]), file);
                fputc(clampColor(v[2]), file);
            }
        fclose(file);
    }

    static Image *loadTGA(const char *filename) {
        assert(filename != NULL);

        // Must end in .tga
        const char *ext = &filename[strlen(filename) - 4];
        assert(!strcmp(ext, ".tga"));

        FILE *file = fopen(filename, "rb");

        auto readByte = [&](FILE *file) -> unsigned char {
            unsigned char b;
            int success = fread((void *)(&b), sizeof(unsigned char), 1, file);
            assert(success == 1);
            return b;
        };

        // Misc header information
        int width = 0;
        int height = 0;
        for (int i = 0; i < 18; i++) {
            unsigned char tmp;
            tmp = readByte(file);
            if (i == 2) assert(tmp == 2);
            else if (i == 12) width += tmp;
            else if (i == 13) width += 256 * tmp;
            else if (i == 14) height += tmp;
            else if (i == 15) height += 256 * tmp;
            else if (i == 16) assert(tmp == 24);
            else if (i == 17) assert(tmp == 32);
            else assert(tmp == 0);
        }

        // Data
        Image *answer = new Image(width, height);

        // Flip y so that (0, 0) is bottom left corner
        for (int y = height - 1; y >= 0; y--)
            for (int x = 0; x < width; x++) {
                unsigned char r, g, b;
                // Note reversed order: b, g, r
                b = readByte(file);
                g = readByte(file);
                r = readByte(file);
                Vector3f color(r / 255.0, g / 255.0, b / 255.0);
                answer->setPixel(x, y, color);
            }
        fclose(file);
        return answer;
    }

    void saveTGA(const char *filename) const {
        assert(filename != NULL);

        // Must end in .tga
        const char* ext = &filename[strlen(filename) - 4];
        assert(!strcmp(ext, ".tga"));

        FILE* file = fopen(filename, "wb");

        auto writeByte = [&](FILE *file, unsigned char b) {
            int success = fwrite((void *)(&b), sizeof(unsigned char), 1, file);
            assert(success == 1);
        };

        // Misc header information
        for (int i = 0; i < 18; i++) {
            if (i == 2) writeByte(file, 2);
            else if (i == 12) writeByte(file, width % 256);
            else if (i == 13) writeByte(file, width / 256);
            else if (i == 14) writeByte(file, height % 256);
            else if (i == 15) writeByte(file, height / 256);
            else if (i == 16) writeByte(file, 24);
            else if (i == 17) writeByte(file, 32);
            else writeByte(file, 0);
        }

        // Data
        // Flip y so that (0, 0) is bottom left corner
        auto clampColor = [&](double c) -> unsigned char {
            int t = int(c * 255);

            if (t < 0) return (unsigned char) 0;
            else if (t > 255) return (unsigned char) 255;
            else return (unsigned char) t;
        };
        for (int y = height - 1; y >= 0; y--)
            for (int x = 0; x < width; x++) {
                Vector3f v = getPixel(x, y);
                // Note reversed order: b, g, r
                writeByte(file, clampColor(v[2]));
                writeByte(file, clampColor(v[1]));
                writeByte(file, clampColor(v[0]));
            }
        fclose(file);
    }

    int saveBMP(const char *filename) {
        int i, j, ipos;
        int bytesPerLine;
        unsigned char *line;
        Vector3f *rgb = data;
        FILE *file;
        struct BMPHeader bmph;

        /* The length of each line must be a multiple of 4 bytes */
        bytesPerLine = (3 * (width + 1) / 4) * 4;

        strcpy(bmph.bfType, "BM");
        bmph.bfOffBits = 54;
        bmph.bfSize = bmph.bfOffBits + bytesPerLine * height;
        bmph.bfReserved = 0;
        bmph.biSize = 40;
        bmph.biWidth = width;
        bmph.biHeight = height;
        bmph.biPlanes = 1;
        bmph.biBitCount = 24;
        bmph.biCompression = 0;
        bmph.biSizeImage = bytesPerLine * height;
        bmph.biXPelsPerMeter = 0;
        bmph.biYPelsPerMeter = 0;
        bmph.biClrUsed = 0;       
        bmph.biClrImportant = 0; 

        file = fopen(filename, "wb");
        if (file == NULL) return 0;
    
        fwrite(&bmph.bfType, 2, 1, file);
        fwrite(&bmph.bfSize, 4, 1, file);
        fwrite(&bmph.bfReserved, 4, 1, file);
        fwrite(&bmph.bfOffBits, 4, 1, file);
        fwrite(&bmph.biSize, 4, 1, file);
        fwrite(&bmph.biWidth, 4, 1, file);
        fwrite(&bmph.biHeight, 4, 1, file);
        fwrite(&bmph.biPlanes, 2, 1, file);
        fwrite(&bmph.biBitCount, 2, 1, file);
        fwrite(&bmph.biCompression, 4, 1, file);
        fwrite(&bmph.biSizeImage, 4, 1, file);
        fwrite(&bmph.biXPelsPerMeter, 4, 1, file);
        fwrite(&bmph.biYPelsPerMeter, 4, 1, file);
        fwrite(&bmph.biClrUsed, 4, 1, file);
        fwrite(&bmph.biClrImportant, 4, 1, file);
    
        line = (unsigned char *) malloc(bytesPerLine);
        if (line == NULL) {
            fprintf(stderr, "Can't allocate memory for BMP file.\n");
            return 0;
        }

        auto clampColor = [&](double c) -> unsigned char {
            int t = int(c * 255);

            if (t < 0) return (unsigned char) 0;
            else if (t > 255) return (unsigned char) 255;
            else return (unsigned char) t;
        };

        for (i = 0; i < height ; i++) {
            for (j = 0; j < width; j++) {
                ipos = (width * i + j);
                line[3 * j] = clampColor(rgb[ipos][2]);
                line[3 * j + 1] = clampColor(rgb[ipos][1]);
                line[3 * j + 2] = clampColor(rgb[ipos][0]);
            }
            fwrite(line, bytesPerLine, 1, file);
        }

        free(line);
        fclose(file);

        return 1;
    }

    void saveImage(const char *filename) {
        int len = strlen(filename);
        if (strcmp(".bmp", filename + len - 4) == 0) {
            saveBMP(filename);
        } else {
            saveTGA(filename);
        }
    }
};