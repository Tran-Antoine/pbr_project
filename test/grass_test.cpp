#include <core/common.h>
#include <stats/perlin.h>
#include <gui/bitmap.h>
#include <ImfArray.h>
#include <ImfRgbaFile.h>
#include <stats/fbm.h>

using namespace nori;

static uint64_t SEED_STATE = 329874;
static uint64_t SEED_SEQ = 103953;

static int IMAGE_SIZE = 512;
static int RES  = 8;

static std::string PATH = "./scenes/ptextures/grass0.exr";


int main(int argc, char **argv) {

    FBM gen(RES, RES, 5, 0.25f, 2.0f, SEED_STATE, SEED_SEQ);
    gen.init_generators();

    Imf::Array2D<Imf::Rgba> out(IMAGE_SIZE, IMAGE_SIZE);


    std::cout << "Calculating perlin noise...\n";
    for(int z = 0; z < IMAGE_SIZE; ++z) {
        for(int x = 0; x < IMAGE_SIZE; ++x) {
            float x_coord = ((float)x) / IMAGE_SIZE;
            float z_coord = ((float)z) / IMAGE_SIZE;

            Color3f light = Color3f(0.337f, 0.635f, 0.039f);
            Color3f dark = Color3f(0.29411765f, 0.4627451f, 0.11372549f);

            float y = gen.get(x_coord, z_coord);

            Color3f modified = 0.5 * (dark * (1-2*y) + light * (1+2*y));

            out[z][x] = Imf::Rgba(modified.r(), modified.g(), modified.b());
        }
    }

    Imf::RgbaOutputFile fileOut(PATH.c_str(), IMAGE_SIZE, IMAGE_SIZE, Imf::WRITE_RGBA);
    fileOut.setFrameBuffer(&out[0][0], 1, IMAGE_SIZE);
    fileOut.writePixels(IMAGE_SIZE);
    std::cout << "File successfully written\n";

}

