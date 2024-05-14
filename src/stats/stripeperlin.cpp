#include <core/common.h>
#include <stats/stripeperlin.h>

NORI_NAMESPACE_BEGIN

void StripePerlinGenerator::init_gradients() {

    gradients.resize(res_x * res_z);

    // init top layer
    for(int x = 0; x < res_x; ++x) {
        Vector2f p = Vector2f(2*random.nextFloat() - 1, 2*random.nextFloat() - 1).normalized();
        gradients[x] = p;
    }

    for(int z = 1; z < res_z; ++z) {
        for(int x = 0; x < res_x; ++x) {
            Vector2f previous = gradients[x + (z - 1) * res_x];
            Vector2f noise = Vector2f(2*random.nextFloat() - 1, 2*random.nextFloat() - 1).normalized();
            gradients[x + z * res_x] = (strength * previous + (1 - strength) * noise).normalized();
        }
    }
}

NORI_NAMESPACE_END