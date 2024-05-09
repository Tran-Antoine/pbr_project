#include <core/common.h>
#include <core/color.h>
#include <bsdf/blendmap.h>
#include <ImfRgbaFile.h>
#include <algorithm>
#include <parser/imageutil.h>

NORI_NAMESPACE_BEGIN

BlendMap::BlendMap(const PropertyList& propList) : DiffuseMap(propList) {

    load_from_file(propList.getString("filename"), pixels);
    width = pixels.width();
    height = pixels.height();
}

Color3f BlendMap::T(float s, float t) {

    if(s > 1 || t > 1 || s < 0 || t < 0) {
        throw NoriException("Texture coordinates out of range [0, 1]");
    }

    int h_index = std::min(height - 1, (int) (t * height));
    int w_index = std::min(width - 1, (int) (s * width));
    const Imf::Rgba &pixel = pixels[h_index][w_index];
    Color3f color = Color3f(pixel.r, pixel.g, pixel.b).toLinearRGB();

    float sum = color.r() + color.g() + color.b();

    Color3f red_color = red_map ? red_map->T(s, t) : 0.f;
    Color3f green_color = green_map ? green_map->T(s, t) : 0.f;
    Color3f blue_color = blue_map ? blue_map->T(s, t) : 0.f;


    return (color.r() * red_color + color.g() * green_color + color.b() * blue_color) / sum;
}


std::string BlendMap::toString() const {
    return "blendmap";
}

void BlendMap::addChild(nori::NoriObject *obj) {
    DiffuseMap* map;
    switch (obj->getClassType()) {
        case NoriObject::EDiffuseMap:
            map = static_cast<DiffuseMap *>(obj);
            if(map->getId() == "red") red_map = map;
            if(map->getId() == "green") green_map = map;
            if(map->getId() == "blue") blue_map = map;

            break;
        default:
            throw NoriException("Mesh::addChild(<%s>) is not supported!",
                                classTypeName(obj->getClassType()));
    }
}

NORI_REGISTER_CLASS(BlendMap, "blend");
NORI_NAMESPACE_END