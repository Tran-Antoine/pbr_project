#pragma once

#include <core/common.h>
#include <pcg32.h>
#include <core/vector.h>

NORI_NAMESPACE_BEGIN

class PerlinGenerator {

public:

    PerlinGenerator(int res_x, int res_z, uint64_t initstate, uint64_t initseq, int n_octaves=3) 
        : res_x(res_x), res_z(res_z), initstate(initstate), initseq(initseq), n_octaves(n_octaves) {
        
        random.seed(initstate, initseq);
    }

    float get(float x, float z) const;
    void init_gradients();
    Vector2f random_gradient(int x, int z) const { return gradients[x + res_x * z]; }

protected:
    int res_x, res_z;
    int n_octaves;
    uint64_t initstate, initseq;

    pcg32 random;    
    std::vector<Vector2f> gradients;
    
};


NORI_NAMESPACE_END