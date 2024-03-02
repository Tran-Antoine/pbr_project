#pragma once

#include <core/common.h>
#include <ImfArray.h>
#include <ImfRgba.h>
#include <core/color.h>

NORI_NAMESPACE_BEGIN

class MipMap {

public:

    enum Quadrant {
        TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT, UNDEFINED
    };

    static Quadrant quadrant(bool top, bool left);
    static void move(Point2i& p, const Quadrant& quadrant);

    MipMap(const std::string &path, const std::string &ext, bool norm);

    void h_distribution(uint8_t depth, Quadrant previous, float &left, float &right) const;
    void v_distribution(uint8_t depth, Quadrant previous, float &up, float &down) const;

    void distribution(uint8_t depth, Point2i previous_pos, Point2i next_corner, float& left, float& right, float&up, float& down) const;

    float get_luminance() const { return total_luminance; }
    bool is_normalized() const { return norm; }
    int max_resolution() const { return max_res; }
    void write_exr() const;
    float grayscale(int x, int y) const;

    uint8_t depth() const { return max_depth; }

protected:

    void corner(uint8_t depth, int& index_x, int& index_y) const;
    void shrink(Imf::Array2D<float>& array, int size_x, int size_y);
    Color3f color(int x, int y) const;

    Imf::Array2D<Imf::Rgba> original;
    Imf::Array2D<float> map;
    int max_res;
    int max_depth;
    bool norm;
    float total_luminance;
};

NORI_NAMESPACE_END
