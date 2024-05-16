#include <iostream>
#include <core/vector.h>
#include <volume/procedural.h>

static std::string PATH = "assets/voxel/procedural/spiral1.vdb";

using namespace nori;

int main() {
    write_spiral(Vector3i(120, 40, 120), Transform(),PATH);
}