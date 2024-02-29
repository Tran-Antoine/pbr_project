#include <core/common.h>
#include <ImfArray.h>
#include <ImfRgbaFile.h>
#include <opencv2/opencv.hpp>

NORI_NAMESPACE_BEGIN

void load_from_file(const std::string& filename, Imf::Array2D<Imf::Rgba>& pixels);

void load_from_file(const std::string& path, const std::string& ext, Imf::Array2D<Imf::Rgba>& pixels);

NORI_NAMESPACE_END