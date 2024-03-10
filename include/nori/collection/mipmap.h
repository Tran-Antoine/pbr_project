#pragma once

#include <core/common.h>
#include <core/vector.h>
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

    MipMap(const std::string filename, bool norm) : 
        MipMap(getFileResolver()->resolve(filename).str(), 
        getFileResolver()->resolve(filename).extension(), 
        norm) {}
        
    MipMap(const std::string &path, const std::string &ext, bool norm);

    void h_distribution(Point2i corner, bool down, float &left, float &right) const;
    void v_distribution(Point2i corner, float &up, float &down) const;

    Point2i next_corner(uint8_t next_depth, Point2i previous_pos) const;

    float get_luminance() const { return total_luminance; }
    bool is_normalized() const { return norm; }
    int max_resolution() const { return max_res; }
    int max_index() const { return max_res - 1; }
    float max_param() const { return (float) max_resolution() - 1.f; }
    void write_exr(const std::string& path) const;
    float grayscale(int x, int y) const;

    int depth() const { return max_depth; }
    Color3f color(float x, float y, bool lerp) const;
    Color3f color(const Point2f &p, bool lerp) const { return color(p.x(), p.y(), lerp); }

protected:

    void corner(uint8_t depth, int& index_x, int& index_y) const;
    void shrink(Imf::Array2D<float>& array, int size_x, int size_y);

    Imf::Array2D<Imf::Rgba> original;
    Imf::Array2D<float> map;
    int max_res;
    int max_depth;
    bool norm;
    float total_luminance;
};

NORI_NAMESPACE_END
