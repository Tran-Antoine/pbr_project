#include <iostream>
#include <volume/coefficient.h>
#include <core/transform.h>
#include <core/vector.h>

static std::string PATH = "assets/voxel/Cloud_02.vdb";

using namespace nori;

int main() {

    VoxelReader cloud(PATH, Transform());

    std::cout << "Majorant: " << cloud.get_majorant() << "\n";
    std::cout << "Rdm value: " << cloud.eval(Point3f(-0.1f, 0.2f, 0.4f), 0.f) << "\n";
}