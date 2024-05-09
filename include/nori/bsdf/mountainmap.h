#pragma once

#include <core/common.h>
#include <bsdf/graymap.h>
#include <bsdf/diffusemap.h>

NORI_NAMESPACE_BEGIN

class MountainMap : public GrayMap {

public:
    explicit MountainMap(DiffuseMap* child, float grass_thr, float rock_thr)
            : GrayMap(child), grass_thr(grass_thr), rock_thr(rock_thr) {}

    Color3f map(float gray) override;

private:
    float grass_thr, rock_thr;
};

NORI_NAMESPACE_END
