#pragma once

#include <core/common.h>
#include <ImfArray.h>
#include <ImfRgba.h>
#include <core/color.h>

NORI_NAMESPACE_BEGIN

class MipMap {

public:
    explicit MipMap(const std::string &path, const std::string &ext);

    void h_distribution(uint8_t depth, int x, int y, float& left, float& right);
    void v_distribution(uint8_t depth, int x, int y, float& up, float&down);
    void write_exr();

protected:

    void corner(uint8_t depth, int& index_x, int& index_y) const;
    float grayscale(int x, int y) const;
    Color3f color(int x, int y) const;

    Imf::Array2D<Imf::Rgba> original;
    Imf::Array2D<float> map;
    int max_res;
    int max_depth;
};

NORI_NAMESPACE_END
