#include <core/common.h>
#include <ImfArray.h>
#include <ImfRgbaFile.h>
#include <core/vector.h>
#include <gui/bitmap.h>
#include <core/color.h>

using namespace nori;

static std::string PATH_1 = "./renders/final/256fullres2.exr";
static std::string PATH_2 = "./renders/final/256fullres2b.exr";
static std::string PATH_OUTPUT = "./renders/final/512fullres2.exr";



int main(int argc, char **argv) {

    Imf::RgbaInputFile file1(PATH_1.c_str());
    Imath::Box2i dw1 = file1.dataWindow();
    long width1 = dw1.max.x - dw1.min.x + 1;
    long height1 = dw1.max.y - dw1.min.y + 1;

    Imf::RgbaInputFile file2(PATH_1.c_str());
    Imath::Box2i dw2 = file1.dataWindow();
    long width2 = dw2.max.x - dw2.min.x + 1;
    long height2 = dw2.max.y - dw2.min.y + 1;

    if(width1 != width2 || height1 != height2) {
        std::cout << "File sizes do not match" << std::endl;
        return -1;
    }

    long width = width1, height = height1;

    Imf::Array2D<Imf::Rgba> merged(height, width);
    Imf::Array2D<Imf::Rgba> image1(height, width);
    Imf::Array2D<Imf::Rgba> image2(height, width);

    file1.setFrameBuffer(&image1[0][0] - dw1.min.x - dw1.min.y * width, 1, width);
    file1.readPixels(dw1.min.y, dw1.max.y);
    file2.setFrameBuffer(&image2[0][0] - dw2.min.x - dw2.min.y * width, 1, width);
    file2.readPixels(dw2.min.y, dw2.max.y);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const Imf::Rgba p1 = image1[y][x];
            const Imf::Rgba p2 = image2[y][x];
            const Imf::Rgba m = Imf::Rgba((p1.r + p2.r) / 2, (p1.g + p2.g) / 2, (p1.b + p2.b) / 2, (p1.a + p2.a) / 2);
            merged[y][x] = m;
        }
    }

    Imf::RgbaOutputFile output(PATH_OUTPUT.c_str(), width, height, Imf::WRITE_RGBA);
    output.setFrameBuffer(&merged[0][0], 1, width);
    output.writePixels(height);
}