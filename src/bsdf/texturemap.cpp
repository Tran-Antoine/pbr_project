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
}

Color3f TextureDiffuseMap::T(float s, float t) const {

    if(s > 1 || t > 1 || s < 0 || t < 0) {
        throw NoriException("Texture coordinates out of range [0, 1]");
    }

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