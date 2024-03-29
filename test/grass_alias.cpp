#include <core/common.h>
#include <stats/perlin.h>
#include <gui/bitmap.h>
#include <ImfArray.h>
#include <ImfRgbaFile.h>
#include <stats/fbm.h>
#include <bsdf/texturemap.h>

using namespace nori;

static uint64_t SEED_STATE = 389874;
static uint64_t SEED_SEQ = 108393;

static int IMAGE_SIZE = 2048;
static int RES = 8;

static std::string OUT_TEXTURE = "./scenes/ptextures/multi_texture.exr";
static std::string OUT_ALIAS = "./scenes/ptextures/grass_alias.exr";

static std::string TEXTURE_1 = "./scenes/ptextures/sandrock.jpg";
static std::string TEXTURE_2 = "./scenes/ptextures/path.jpg";

static float steepLerp(float t, float a, float b) {
    float s;
    if(t > 0.65f) {
        s = 1.f;
    } else if(t < 0.35f) {
        s = 0.f;
    } else {
        s = (t - 0.35f) / 0.3f;
    }
    return lerp(s, a, b);
}
static Color3f lerp(float t, Color3f c1, Color3f c2) {
    float r = steepLerp(t, c1.r(), c2.r());
    float g = steepLerp(t, c1.g(), c2.g());
    float b = steepLerp(t, c1.b(), c2.b());
    return Color3f(r,g,b);
}

int main(int argc, char **argv) {

    FBM alias(RES, RES, 3, 0.25f, 2.0f, SEED_STATE, SEED_SEQ);
    alias.init_generators();

    Imf::Array2D<Imf::Rgba> out_texture(IMAGE_SIZE, IMAGE_SIZE);
    Imf::Array2D<Imf::Rgba> out_alias(IMAGE_SIZE, IMAGE_SIZE);

    TextureDiffuseMap text1(TEXTURE_1);
    TextureDiffuseMap text2(TEXTURE_2);


    std::cout << "Calculating perlin noise...\n";

    float max = -1, min = 1;

    for(int z = 0; z < IMAGE_SIZE; ++z) {
        for(int x = 0; x < IMAGE_SIZE; ++x) {
            float x_coord = ((float)x) / IMAGE_SIZE;
            float z_coord = ((float)z) / IMAGE_SIZE;

            float y = alias.get(x_coord, z_coord);
            if(y < min) min = y;
            if(y > max) max = y;
        }
    }

    for(int z = 0; z < IMAGE_SIZE; ++z) {
        for(int x = 0; x < IMAGE_SIZE; ++x) {
            float x_coord = ((float)x) / IMAGE_SIZE;
            float z_coord = ((float)z) / IMAGE_SIZE;

            float y = alias.get(x_coord, z_coord);
            Color3f c1 = text1.T(x_coord, z_coord);
            Color3f c2 = text2.T(x_coord, z_coord);

            float t = (y - min) / (max - min);

            Color3f color = lerp(t, c1, c2);

            out_texture[z][x] = Imf::Rgba(color.r(), color.g(), color.b());
            out_alias[z][x] = Imf::Rgba(t,t,t);
        }
    }

    Imf::RgbaOutputFile fileOut(OUT_TEXTURE.c_str(), IMAGE_SIZE, IMAGE_SIZE, Imf::WRITE_RGBA);
    fileOut.setFrameBuffer(&out_texture[0][0], 1, IMAGE_SIZE);
    fileOut.writePixels(IMAGE_SIZE);

    Imf::RgbaOutputFile fileOut2(OUT_ALIAS.c_str(), IMAGE_SIZE, IMAGE_SIZE, Imf::WRITE_RGBA);
    fileOut2.setFrameBuffer(&out_alias[0][0], 1, IMAGE_SIZE);
    fileOut2.writePixels(IMAGE_SIZE);
    std::cout << "Files successfully written\n";

}

