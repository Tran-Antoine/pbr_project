#include <core/common.h>
#include <core/color.h>
#include <bsdf/texturemap.h>
#include <ImfRgbaFile.h>
#include <algorithm>
#include <parser/imageutil.h>

NORI_NAMESPACE_BEGIN

TextureDiffuseMap::TextureDiffuseMap(const PropertyList &propList) : DiffuseMap(propList) {

    load_from_file(propList.getString("filename"), pixels);
    width = pixels.width();
    height = pixels.height();
    repeat = propList.getFloat("repeat", 1.0f);
}

Color3f TextureDiffuseMap::T(float s, float t) {

    if(s > 1 + Epsilon || t > 1 + Epsilon || s < -Epsilon || t < -Epsilon) {
        throw NoriException("Texture coordinates out of range [0, 1]");
    }

    s = clamp(s, 0.f, 1.f);
    t = clamp(t, 0.f, 1.f);

    s -= ((int) (s / repeat)) * repeat;
    t -= ((int) (t / repeat)) * repeat;

    int h_index = std::min(height - 1, (int) (t * height));
    int w_index = std::min(width - 1, (int) (s * width));
    const Imf::Rgba &pixel = pixels[h_index][w_index];

    return Color3f(pixel.r, pixel.g, pixel.b).toLinearRGB();
}

std::string TextureDiffuseMap::toString() const {
    return "texturemap";
}

NORI_REGISTER_CLASS(TextureDiffuseMap, "texture");
NORI_NAMESPACE_END