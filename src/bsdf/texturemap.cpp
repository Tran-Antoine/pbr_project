#include <core/common.h>
#include <core/color.h>
#include <bsdf/diffusemap.h>
#include <bsdf/texturemap.h>
#include <ImfRgbaFile.h>
#include <ImfArray.h>

NORI_NAMESPACE_BEGIN

TextureDiffuseMap::TextureDiffuseMap(std::string filename) {

    filesystem::path path = getFileResolver()->resolve(filename);

    Imf::RgbaInputFile file(path.str().c_str());
    Imath::Box2i dw = file.dataWindow();
    width = dw.max.x - dw.min.x + 1;
    height = dw.max.y - dw.min.y + 1;

    
    pixels.resizeErase(height, width);
    file.setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
    file.readPixels(dw.min.y, dw.max.y);
}

TextureDiffuseMap::~TextureDiffuseMap() {
    
}

Color3f TextureDiffuseMap::T(float s, float t) const {

    int h_index = std::floorf(t * height);
    int w_index = std::floorf(s * width);
    const Imf::Rgba &pixel = pixels[h_index][w_index];

    return Color3f(pixel.r, pixel.g, pixel.b);
}

NORI_NAMESPACE_END