#include <core/common.h>
#include <core/color.h>
#include <bsdf/texturemap.h>
#include <ImfRgbaFile.h>
#include <algorithm>
#include <opencv2/opencv.hpp>

NORI_NAMESPACE_BEGIN

TextureDiffuseMap::TextureDiffuseMap(const std::string& filename) {

    filesystem::path path = getFileResolver()->resolve(filename);
    std::string ext = path.extension();

    if(ext == "exr") {
        Imf::RgbaInputFile file(path.str().c_str());
        Imath::Box2i dw = file.dataWindow();
        width = dw.max.x - dw.min.x + 1;
        height = dw.max.y - dw.min.y + 1;

        pixels.resizeErase(height, width);
        file.setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
        file.readPixels(dw.min.y, dw.max.y);
    } else if(ext == "png" || ext == "jpg") {

        cv::Mat image = cv::imread(path.str());

        if(image.empty()) {
            throw NoriException("The image could not be found");
        }

        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);

        pixels.resizeErase(image.rows, image.cols);
        width = image.cols;
        height = image.rows;

        for (int y = 0; y < image.rows; ++y) {
            for (int x = 0; x < image.cols; ++x) {
                cv::Vec3b pixel = image.at<cv::Vec3b>(y, x);
                float r = static_cast<float>(pixel[0]) / 255.f;
                float g = static_cast<float>(pixel[1]) / 255.f;
                float b = static_cast<float>(pixel[2]) / 255.f;
                pixels[y][x] = Imf::Rgba(r, g, b);
            }
        }

    } else {
        throw NoriException("The file extension is not supported");
    }

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

NORI_NAMESPACE_END