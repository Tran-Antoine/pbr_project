#pragma once

#include <core/common.h>
#include <stats/perlin.h>

NORI_NAMESPACE_BEGIN

class FBM {
public:

    FBM(int base_freq_x, int base_freq_z, int nOctaves, float ampDecay, float freqIncrease, uint64_t initstate,
        uint64_t initseq)
            : base_freq_x(base_freq_x), base_freq_z(base_freq_z), n_octaves(nOctaves), amp_decay(ampDecay),
              freq_increase(freqIncrease), initstate(initstate), initseq(initseq) {}

    ~FBM() {
        for(auto p : perlins) {
            delete p;
        }
    }
    void init_generators();
    float get(float x, float z) const;

protected:

    int base_freq_x, base_freq_z;
    uint64_t initstate, initseq;
    float amp_decay, freq_increase;
    int n_octaves;

    std::vector<PerlinGenerator*> perlins;
};

NORI_NAMESPACE_END
