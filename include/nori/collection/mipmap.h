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

    //MipMap() {};
    MipMap(const std::string filename, bool norm) : 
        MipMap(getFileResolver()->resolve(filename).str(), 
        getFileResolver()->resolve(filename).extension(), 
        norm) {}
        
    MipMap(const std::string &path, const std::string &ext, bool norm);

    void h_distribution(Point2i corner, bool down, float &left, float &right) const;
    void v_distribution(Point2i corner, float &up, float &down) const;

    Point2i next_corner(uint8_t next_depth, Point2i previous_pos) const;

    void distribution(uint8_t depth, Point2i corner, float& left, float& right, float&up, float& down) const;

    float get_luminance() const { return total_luminance; }
    bool is_normalized() const { return norm; }
    int max_resolution() const { return max_res; }
    void write_exr() const;
    float grayscale(int x, int y) const;

    int depth() const { return max_depth; }
    Color3f color(int x, int y) const;
    Color3f color(const Point2i& p) const { return color(p.x(), p.y()); }
    Color3f color(const Point2f& p) const { return color((int) p.x(), (int) p.y()); }

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
