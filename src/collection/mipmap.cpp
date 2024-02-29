#include <core/common.h>
#include <collection/mipmap.h>
#include <parser/imageutil.h>
#include <gui/bitmap.h>

static float gray(const Imf::Rgba& pixel) {
    return (pixel.r + pixel.g + pixel.b) / 3.f;
}

static void shrink(Imf::Array2D<float>& array, int size_x, int size_y) {
    for(int y = 0; y < size_y; y += 2) {
        for(int x = 0; x < size_x; x += 2) {
            float tl = array[y][x];
            float tr = array[y][x+1];
            float bl = array[y+1][x];
            float br = array[y+1][x+1];
            array[y/2][x/2] = tl + tr + bl + br;
        }
    }
}

nori::MipMap::MipMap(const std::string &path, const std::string &ext) {

    using GrayMap = Imf::Array2D<float>;

    load_from_file(path, ext,original);
    max_res = original.height();

    if((max_res & (max_res - 1)) != 0) {
        throw NoriException("Dimensions must be a power of 2");
    }

    // TODO: figure out what the heck the height and width are connected to

    max_depth = (int) log2(max_res); // not very efficient but it doesn't matter
    map.resizeErase(max_res, max_res + max_res / 2);

    GrayMap temp(max_res, max_res);

    float total = 0;
    for(int y = 0; y < max_res; y++) {
        for(int x = 0; x < max_res; x++) {
            float gray_value = gray(original[y][x]);
            map[y][x] = gray_value;
            temp[y][x] = gray_value;
            total += gray_value;
        }
    }
    // Normalizing step
    for(int y = 0; y < max_res; y++) {
        for(int x = 0; x < max_res; x++) {
            map[y][x] /= total;
            temp[y][x] /= total;
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

void nori::MipMap::h_distribution(uint8_t depth, int x, int y, float &left, float &right) {
    int index_x, index_y;
    corner(depth, index_x, index_y);

    index_x += x;
    index_y += y;

    /*float left_total = grayscale(index_x, index_y) + grayscale(index_x, index_y + 1);
    float right_total = grayscale(index_x + 1, index_y) + grayscale(index_x + 1, index_y + 1);

    left = left_total / (left_total + right_total);
    right = 1 - left;*/
    // above step not needed as the map is normalized
    left = grayscale(index_x, index_y) + grayscale(index_x, index_y + 1);
    right = 1 - left;
}

void nori::MipMap::v_distribution(uint8_t depth, int x, int y, float &up, float &down) {
    int index_x, index_y;
    corner(depth, index_x, index_y);

    index_x += x;
    index_y += y;

    /*
    float up_total = grayscale(index_x, index_y) + grayscale(index_x + 1, index_y);
    float down_total = grayscale(index_x, index_y + 1) + grayscale(index_x + 1, index_y + 1);

    up = up_total / (up_total + down_total);
    down = 1 - up;*/
    // above step not needed as the map is normalized
    up = grayscale(index_x, index_y) + grayscale(index_x + 1, index_y);
    down = 1 - up;
}

void nori::MipMap::corner(uint8_t depth, int &index_x, int &index_y) const {
    if(depth == max_depth) {
        index_x = 0;
        index_y = 0;
        return;
    }
    index_x = max_res;
    index_y = max_res - ((1 << (depth+1)));
}

float nori::MipMap::grayscale(int x, int y) const {
    if(x < 0 || y < 0 || x >= map.width() || y >= map.height()) {
        throw NoriException("Grayscale indices out of range");
    }
    return map[y][x];
}

nori::Color3f nori::MipMap::color(int x, int y) const {
    Imf::Rgba pixel = original[y][x];
    return Color3f(pixel.r, pixel.g, pixel.b);
}

void nori::MipMap::write_exr() {

    std::cout << map.width() << " " << map.height();
    Bitmap out(Vector2i(map.width(), map.height()));

    for(int y = 0; y < map.height(); ++y) {
        for(int x = 0; x < map.width(); ++x) {
            out.coeffRef(y, x) = map[y][x];
        }
    }

    std::cout << "Bitmap ready to be written\n";
    out.saveEXR("scenes/ibl/mipmap");
    out.savePNG("scenes/ibl/mipmap");
}


