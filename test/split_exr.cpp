#include <core/common.h>
#include <ImfArray.h>
#include <ImfRgbaFile.h>
#include <core/vector.h>
#include <gui/bitmap.h>
#include <core/color.h>

using namespace nori;

static std::string PATH = R"(.\scenes\ibl\sky.exr)";


int main(int argc, char **argv) {

    Imf::RgbaInputFile file(PATH.c_str());
    Imath::Box2i dw = file.dataWindow();
    long width = dw.max.x - dw.min.x + 1;
    long height = dw.max.y - dw.min.y + 1;

    Imf::Array2D<Color3f> left;
    Imf::Array2D<Color3f> right;
    Imf::Array2D<Imf::Rgba> pixels;

    pixels.resizeErase(width, height);
    left.resizeErase(width/2, height);
    right.resizeErase(width/2, height);

    file.setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
    file.readPixels(dw.min.y, dw.max.y);

    for(int y = 0; y < height; ++y) {
        for(int x = 0; x < width/2; ++x) {
            const Imf::Rgba& pixel = pixels[y][x];
            left[y][x] = Color3f(pixel.r, pixel.g, pixel.b);
        }
        for(int x = width/2; x < width; ++x) {
            const Imf::Rgba& pixel = pixels[y][x];
            right[y][x-width/2] = Color3f(pixel.r, pixel.g, pixel.b);
        }
    }

    Bitmap left_out(Vector2i(left.width(), left.height()));

    for(int y = 0; y < left.height(); ++y) {
        for(int x = 0; x < left.width(); ++x) {
            left_out.coeffRef(y, x) = left[y][x];
        }
    }

    Bitmap right_out(Vector2i(left.width(), left.height()));

    for(int y = 0; y < left.height(); ++y) {
        for(int x = 0; x < left.width(); ++x) {
            right_out.coeffRef(y, x) = left[y][x];
        }
    }

    left_out.saveEXR("left");
    right_out.saveEXR("right");

}