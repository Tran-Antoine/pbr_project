#include <core/common.h>
#include <core/color.h>
#include <bsdf/luminosity.h>

NORI_NAMESPACE_BEGIN

RandomLuminosityDiffuseMap::RandomLuminosityDiffuseMap(const PropertyList& propList)
    : DiffuseMap(propList),
    hue(propList.getInteger("hue")),
    saturation(propList.getFloat("saturation")),
    min_lum(propList.getFloat("min-lum")),
    max_lum(propList.getFloat("max-lum")) {}

Color3f RandomLuminosityDiffuseMap::T(float s, float t) {

    float L = min_lum + random.nextFloat() * (max_lum - min_lum);
    float S = saturation, H = hue;

    float C = (1 - abs(2 * L - 1)) * S;
    int H_prime = H / 60.0f;
    float X = C * (1 - abs(H_prime % 2 - 1));

    float R = 0, G = 0, B = 0;

    if (H_prime >= 0 && H_prime < 1) {
        R = C;
        G = X;
    } else if (H_prime >= 1 && H_prime < 2) {
        R = X;
        G = C;
    } else if (H_prime >= 2 && H_prime < 3) {
        G = C;
        B = X;
    } else if (H_prime >= 3 && H_prime < 4) {
        G = X;
        B = C;
    } else if (H_prime >= 4 && H_prime < 5) {
        R = X;
        B = C;
    } else if (H_prime >= 5 && H_prime < 6) {
        R = C;
        B = X;
    }
    float M = L - C / 2;
    Color3f output = Color3f(R + M, G + M, B + M);

    std::cout << output.r() << ", " << output.g() << ", " << output.b() << std::endl;

    return output;
}

std::string RandomLuminosityDiffuseMap::toString() const {
    return "uniform";
}

NORI_REGISTER_CLASS(RandomLuminosityDiffuseMap, "random-luminosity");
NORI_NAMESPACE_END