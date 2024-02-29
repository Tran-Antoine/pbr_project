#include <core/common.h>
#include <collection/mipmap.h>

using namespace nori;

static std::string PATH = R"(.\scenes\ibl\cat.jpg)";


int main(int argc, char **argv) {

    MipMap map(PATH, "jpg", false);
    map.write_exr();
    return 0;
}