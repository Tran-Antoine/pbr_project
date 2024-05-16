#include <core/common.h>
#include <stats/fbm.h>
#include <gui/bitmap.h>
#include <stats/stripeperlin.h>

using namespace nori;

static uint64_t SEED_STATE = 372212178149;
static uint64_t SEED_SEQ = 102822176429;

static int IMAGE_SIZE = 1024;
static int BASE_RES_X  = 4;
static int BASE_RES_Z = 4;

static PerlinGenerator* genFun(int res_x, int res_z, uint64_t initstate, uint64_t initseq) {
    return new StripePerlinGenerator(1, res_x, res_z, initstate, initseq);
}

int main(int argc, char **argv) {

    FBM gen(BASE_RES_X, BASE_RES_Z, 3, 0.3, 2.0f, SEED_STATE, SEED_SEQ);
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

            float y_mapped = ((out.coeffRef(z, x) - min_y) / (max_y - min_y)).x();
            out.coeffRef(z, x) = y_mapped;
        }
    }

    std::cout << "Bitmap ready to be written\n";
    out.savePNG("assets/terrain/terrain5");

    return 0;
}
