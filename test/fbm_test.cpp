#include <core/common.h>
#include <stats/fbm.h>
#include <gui/bitmap.h>

using namespace nori;

static uint64_t SEED_STATE = 329874;
static uint64_t SEED_SEQ = 103953;

static int IMAGE_SIZE = 512;
static int BASE_RES  = 6;

int main(int argc, char **argv) {

    FBM gen(BASE_RES, BASE_RES, 5, 0.25f, 2.0f, SEED_STATE, SEED_SEQ);
    gen.init_generators();

    Bitmap out(Vector2i(IMAGE_SIZE, IMAGE_SIZE));

    std::cout << "Calculating FBM noise...\n";

    float min_y = 100000, max_y = -100000;

    for(int z = 0; z < IMAGE_SIZE; ++z) {
        for(int x = 0; x < IMAGE_SIZE; ++x) {
            float x_coord = ((float)x) / IMAGE_SIZE;
            float z_coord = ((float)z) / IMAGE_SIZE;

            float y = gen.get(x_coord, z_coord);

            if(y > max_y) max_y = y;
            if(y < min_y) min_y = y;

            out.coeffRef(z, x) = y;
        }
    }

    for(int z = 0; z < IMAGE_SIZE; ++z) {
        for(int x = 0; x < IMAGE_SIZE; ++x) {

            auto mapping = (out.coeffRef(z, x) - min_y) / (max_y - min_y);
            // manually add flat valleys
            out.coeffRef(z, x) = std::max(mapping.x() - 0.3f, mapping.x() / 5.f);
        }
    }

    std::cout << "Bitmap ready to be written\n";
    out.saveEXR("scenes/terrain/fbm-heights");
    out.savePNG("fbm");

    return 0;
}
