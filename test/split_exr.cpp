#include <core/common.h>
#include <ImfArray.h>
#include <ImfRgbaFile.h>
#include <core/vector.h>
#include <gui/bitmap.h>
#include <core/color.h>

using namespace nori;

static std::string PATH = "./assets/env/bluesky.exr";


int main(int argc, char **argv) {

    Imf::RgbaInputFile file(PATH.c_str());
    Imath::Box2i dw = file.dataWindow();
    long width = dw.max.x - dw.min.x + 1;
    long height = dw.max.y - dw.min.y + 1;

    Imf::Array2D<Imf::Rgba> left(height, width / 2);
    Imf::Array2D<Imf::Rgba> right(height, width / 2);
    Imf::Array2D<Imf::Rgba> pixels(height, width);

    file.setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
    file.readPixels(dw.min.y, dw.max.y);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width / 2; ++x) {
            left[y][x] = pixels[y][x];
            right[y][x] = pixels[y][x + width / 2];
        }
    }

    Imf::RgbaOutputFile fileLeft("./assets/env/bluesky_left.exr", width / 2, height, Imf::WRITE_RGBA);
    fileLeft.setFrameBuffer(&left[0][0], 1, width / 2);
    fileLeft.writePixels(height);

    Imf::RgbaOutputFile fileRight("./assets/env/bluesky_right.exr", width / 2, height, Imf::WRITE_RGBA);
    fileRight.setFrameBuffer(&right[0][0], 1, width / 2);
    fileRight.writePixels(height);
}