#include <core/common.h>
#include <ImfArray.h>
#include <ImfRgbaFile.h>
#include <parser/imageutil.h>
#include <opencv2/opencv.hpp>

NORI_NAMESPACE_BEGIN

void load_from_file(const std::string& filename, Imf::Array2D<Imf::Rgba>& pixels) {

    filesystem::path path = getFileResolver()->resolve(filename);
    std::string ext = path.extension();

    if(ext == "exr") {
        Imf::RgbaInputFile file(path.str().c_str());
        Imath::Box2i dw = file.dataWindow();
        long width = dw.max.x - dw.min.x + 1;
        long height = dw.max.y - dw.min.y + 1;

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

NORI_NAMESPACE_END