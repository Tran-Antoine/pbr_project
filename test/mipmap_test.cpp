#include <core/common.h>
#include <collection/mipmap.h>

using namespace nori;

static std::string PATH = R"(.\scenes\ibl\test-probe.png)";


int main(int argc, char **argv) {

    MipMap map(PATH, "png");
    map.write_exr();
    return 0;
}