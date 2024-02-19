#include <core/common.h>
#include <stats/perlin.h>

NORI_NAMESPACE_BEGIN

static float fade(float t) {
    return ((6*t - 15)*t + 10)*t*t*t;
}

void PerlinGenerator::init_gradients() {

    gradients.resize(res_x * res_z);

    for(int z = 0; z < res_z; ++z) {
        for(int x = 0; x < res_x; ++x) {
            Vector2f p = Vector2f(2*random.nextFloat() - 1, 2*random.nextFloat() - 1).normalized();
            gradients[x + z * res_x] = std::move(p);
        }
    } 
}

float PerlinGenerator::get(float x, float z) const {

    if(gradients.size() == 0) {
        throw NoriException("Cannot call PerlinGenerator::get before PerlinGenerator::init_gradients");
    }

    if(x < 0 || x >= res_x - 1 || z < 0 || z >= res_z - 1) {
        throw NoriException("Coordinates (x, z) out of range");
    }

    int x0 = (int) x;
    int x1 = x0 + 1;
    int z0 = (int) z;
    int z1 = z0 + 1;

    const Vector2f& g0 = random_gradient(x0, z0);
    const Vector2f& g1 = random_gradient(x0, z1);
    const Vector2f& g2 = random_gradient(x1, z0);
    const Vector2f& g3 = random_gradient(x1, z1);

    Point2f p(x, z);

    const Vector2f& off0 = p - Point2f(x0, z0);
    const Vector2f& off1 = p - Point2f(x0, z1);
    const Vector2f& off2 = p - Point2f(x1, z0);
    const Vector2f& off3 = p - Point2f(x1, z1);

    float dp0 = g0.dot(off0);    
    float dp1 = g1.dot(off1);
    float dp2 = g2.dot(off2);
    float dp3 = g3.dot(off3);

    float t_h = fade(x - x0);
    float t_v = fade(z - z0);

    float v_inter_left  = lerp(t_v, dp0, dp1);
    float v_inter_right = lerp(t_v, dp3, dp2);

    return lerp(t_h, v_inter_left, v_inter_right);
}

NORI_NAMESPACE_END