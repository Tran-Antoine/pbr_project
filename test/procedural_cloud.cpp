#include <iostream>
#include <core/vector.h>
#include <volume/procedural.h>

static std::string PATH = "assets/voxel/procedural/spiral.vdb";

using namespace nori;

int main() {
    write_spiral(Vector3i(150, 40, 150), PATH);
}