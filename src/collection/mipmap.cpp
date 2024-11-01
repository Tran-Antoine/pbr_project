#include <core/common.h>
#include <collection/mipmap.h>
#include <parser/imageutil.h>
#include <gui/bitmap.h>
#include <math.h>

NORI_NAMESPACE_BEGIN

static float gray(const Imf::Rgba& pixel) {
    return ((float)pixel.r + (float)pixel.g + (float)pixel.b) / 3.f;
}

static float cap(float f) {
    if(isinf(f)) {
        return 65504.f;
    }
    return f;
}

static Color3f toColor(const Imf::Rgba& pixel) {
    float r = pixel.r, g = pixel.g, b = pixel.b;
    return Color3f(cap(r), cap(g), cap(b));
}

void MipMap::shrink(Imf::Array2D<float>& array, int size_x, int size_y) {
    for(int y = 0; y < size_y; y += 2) {
        for(int x = 0; x < size_x; x += 2) {
            float tl = array[y][x];
            float tr = array[y][x+1];
            float bl = array[y+1][x];
            float br = array[y+1][x+1];
            array[y/2][x/2] = norm ? tl + tr + bl + br : (tl + tr + bl + br) / 4.f;
        }
    }
}

MipMap::MipMap(const std::string &path, const std::string &ext, bool norm) : norm(norm) {

    using GrayMap = Imf::Array2D<float>;

    load_from_file(path, ext,original);
    max_res = original.height();

    if((max_res & (max_res - 1)) != 0) {
        throw NoriException("Dimensions must be a power of 2");
    }

    max_depth = (int) log2(max_res); // not very efficient but it doesn't matter
    map.resizeErase(max_res, max_res + max_res / 2);

    GrayMap temp(max_res, max_res);

    float total = 0;
    for(int y = 0; y < max_res; y++) {
        for(int x = 0; x < max_res; x++) {
            const Imf::Rgba& pixel = original[y][x];
            Color3f color = toColor(pixel);
            float gray_value = (color.r() + color.g() + color.b()) / 3.f;
            map[y][x] = gray_value;
            temp[y][x] = gray_value;

            total += gray_value;
        }
    }
    total_luminance = total;
    // Normalizing step
    if(norm) {
        for(int y = 0; y < max_res; y++) {
            for(int x = 0; x < max_res; x++) {
                map[y][x] /= total;
                temp[y][x] /= total;
            }
        }
    }

    int current_size = max_res;

    for(int depth = max_depth - 1; depth > 0; depth--) {

        shrink(temp, current_size, current_size);
        current_size /= 2;

        int base_x, base_y;
        corner(depth, base_x, base_y);

        for(int y = 0; y < current_size; y++) {
            for(int x = 0; x < current_size; x++) {
                map[base_y + y][base_x + x] = temp[y][x];
            }
        }

    }
}

void MipMap::corner(uint8_t depth, int &index_x, int &index_y) const {
    if(depth == max_depth) {
        index_x = 0;
        index_y = 0;
        return;
    }
    index_x = max_res;
    index_y = max_res - ((1 << (depth+1)));
}

float MipMap::grayscale(int x, int y) const {
    if(x < 0 || y < 0 || x >= map.width() || y >= map.height()) {
        throw NoriException("Grayscale indices out of range");
    }
    return map[y][x];
}

static Color3f lerp(float dx, float dy, const Color3f& current, const Color3f& right, const Color3f& below) {
    Color3f h_lerp = dx * current + (1 - dx) * right;
    return h_lerp * dy + (1 - dy) * below;
}

Color3f MipMap::color(float x, float y, bool do_lerp) const {

    if(x < -Epsilon || y < -Epsilon || x > 1 + Epsilon || y > 1 + Epsilon) {
        std::cout << "Invalid color indices provided" << std::endl;
        throw NoriException("Invalid color indices provided");
    }

    x = clamp(x, 0.f, 1.f);
    y = clamp(y, 0.f, 1.f);

    float x_mapped = x * max_param();
    float y_mapped = y * max_param();
    int x_index = (int) x_mapped;
    int y_index = (int) y_mapped;
    float dx = x_mapped - (float) x_index, dy = y_mapped - (float) y_index;

    if(do_lerp && x_index < max_index() && y_index < max_index()) {
        Color3f current = toColor(original[y_index][x_index]);
        Color3f right = toColor(original[y_index][x_index+1]);
        Color3f below = toColor(original[y_index+1][x_index]);

        return lerp(dx, dy, current, right, below);
    }

    const Imf::Rgba& pixel = original[y_index][x_index];


    /*float r = std::max(0.f, (float) std::min(pixel.r, std::numeric_limits<half>::max()));
    float g = std::max(0.f, (float) std::min(pixel.g, std::numeric_limits<half>::max()));
    float b = std::max(0.f, (float)std::min(pixel.b, std::numeric_limits<half>::max()));*/
    float r = pixel.r, g = pixel.g, b = pixel.b;

    return Color3f(cap(r), cap(g), cap(b));
}

void MipMap::write_exr(const std::string& path) const {

    Bitmap out(Vector2i(map.width(), map.height()));

    for(int y = 0; y < map.height(); ++y) {
        for(int x = 0; x < map.width(); ++x) {
            out.coeffRef(y, x) = map[y][x];
        }
    }
    out.saveEXR(path);
}

MipMap::Quadrant MipMap::quadrant(bool top, bool left) {
    if(top && left) return Quadrant::TOP_LEFT;
    if(top && !left) return Quadrant::TOP_RIGHT;
    if(!top && left) return Quadrant::BOTTOM_LEFT;
    else return Quadrant::BOTTOM_RIGHT;
}

void MipMap::h_distribution(Point2i corner, bool down, float &left, float &right) const {
    int index_x = corner.x(), index_y = corner.y();

    float left_total = down ? grayscale(index_x, index_y + 1) : grayscale(index_x, index_y);
    float right_total = down ? grayscale(index_x + 1, index_y + 1) : grayscale(index_x + 1, index_y);

    left = left_total / (left_total + right_total);
    right = 1 - left;
}

void MipMap::v_distribution(Point2i corner, float &up, float &down) const {
    int index_x = corner.x(), index_y = corner.y();

    float up_total = grayscale(index_x, index_y) + grayscale(index_x + 1, index_y);
    float down_total = grayscale(index_x, index_y + 1) + grayscale(index_x + 1, index_y + 1);
    
    up = up_total / (up_total + down_total);
    down = 1 - up;
}

Point2i MipMap::next_corner(uint8_t next_depth, Point2i previous_pos) const {
    int index_x, index_y;
    corner(next_depth, index_x, index_y);

    if(next_depth > 1) {
        int prev_corner_x = 0, prev_corner_y = 0;
        corner(next_depth - 1, prev_corner_x, prev_corner_y);
        
        index_x += 2 * (previous_pos.x() - prev_corner_x);
        index_y += 2 * (previous_pos.y() - prev_corner_y);
    }

    return Point2i(index_x, index_y);
}

void MipMap::move(Point2i &p, const MipMap::Quadrant &quadrant) {
    switch(quadrant) {
        case TOP_LEFT: break;
        case TOP_RIGHT: p.x() += 1; break;
        case BOTTOM_LEFT: p.y() += 1; break;
        case BOTTOM_RIGHT: p.x() += 1; p.y() += 1; break;
        case UNDEFINED: break;
    }
}

NORI_NAMESPACE_END

