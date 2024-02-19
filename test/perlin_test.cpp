#include <core/common.h>
#include <stats/perlin.h>
#include <gui/bitmap.h>

using namespace nori;



static uint64_t SEED_STATE = 329874;
static uint64_t SEED_SEQ = 103953;

static int IMAGE_SIZE = 256;
static int RES  = 20;

int main(int argc, char **argv) {

    PerlinGenerator gen(RES, RES, SEED_STATE, SEED_SEQ);
    gen.init_gradients();
    
    Bitmap out(Vector2i(IMAGE_SIZE, IMAGE_SIZE));

    std::cout << "Calculating perlin noise...\n";
    for(int z = 0; z < IMAGE_SIZE; ++z) {
        for(int x = 0; x < IMAGE_SIZE; ++x) {
            float x_coord = RES * ((float)x / IMAGE_SIZE);
            float z_coord = RES * ((float)z / IMAGE_SIZE);

            float y = gen.get(x_coord, z_coord);

            out.coeffRef(z, x) = y;
        }
    }

    std::cout << "Bitmap ready to be written\n";
    out.savePNG("perlin.png");

    return 0;
}

