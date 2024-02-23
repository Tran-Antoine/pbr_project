#include "stats/fbm.h"


void nori::FBM::init_generators() {

    int freq_x = base_freq_x;
    int freq_z = base_freq_z;

    for(int i = 0; i < n_octaves; ++i) {
        auto generator = new PerlinGenerator(freq_x, freq_z, initstate, initseq);
        generator->init_gradients();
        perlins.push_back(generator);

        freq_x = (int) (freq_x * freq_increase);
        freq_z = (int) (freq_z * freq_increase);
    }
}

float nori::FBM::get(float x, float z) const {
    float amplitude = 1.f;
    float freq_x = base_freq_x;
    float freq_z = base_freq_z;

    float result = 0;

    for(auto gen : perlins) {
        float gen_height = amplitude * gen->get(x * (freq_x - 1), z * (freq_z - 1));

        result += gen_height;
        amplitude *= amp_decay;
        freq_x *= freq_increase;
        freq_z *= freq_increase;
    }

    return result;
}
