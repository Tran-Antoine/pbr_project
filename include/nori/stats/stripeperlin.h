#pragma once

#include <core/common.h>
#include <pcg32.h>
#include <core/vector.h>
#include <stats/perlin.h>

NORI_NAMESPACE_BEGIN

class StripePerlinGenerator : public PerlinGenerator {

public:

    StripePerlinGenerator(float strength, int res_x, int res_z, uint64_t initstate, uint64_t initseq)
            : strength(strength),
              PerlinGenerator(res_x, res_z, initstate, initseq) {

        random.seed(initstate, initseq);
    }

    void init_gradients() override;

protected:
    float strength;
};


NORI_NAMESPACE_END
