#include <core/common.h>
#include <bsdf/graymap.h>
#include <bsdf/diffusemap.h>
#include <bsdf/mountainmap.h>

NORI_NAMESPACE_BEGIN

Color3f MountainMap::map(float gray) const {
    if(gray < grass_thr) {
        float max_r = 0.1f;
        float max_g = 0.2f;
        float max_b = 0.0f;
        float f = 0.1f + gray / grass_thr;
        return f * Color3f(max_r, max_g, max_b);
    } else if(gray < rock_thr) {
        float max_r = 0.4f;
        float max_g = 0.3f;
        float max_b = 0.25f;
        float f = 0.2f + (gray - grass_thr) / (rock_thr - grass_thr);
        return f * Color3f(max_r, max_g, max_b);
    } else {
        return Color3f(std::min(1.f, gray + 0.4f));
    }
}

NORI_REGISTER_CLASS(MountainMap, "mountain_map")
NORI_NAMESPACE_END
