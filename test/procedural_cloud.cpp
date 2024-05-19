#include <iostream>
#include <core/vector.h>
#include <volume/procedural.h>

static std::string PATH = "assets/voxel/procedural/sky.vdb";

using namespace nori;

int main() {
    //write_spiral(Vector3i(120, 40, 120), Transform(),PATH);
    write_sky(Vector3i(5, 1, 5), Vector3i(120, 40, 120),
              BoundingBox3f(Point3f(-359, 116, -258), Point3f(330, 138, 233)),
              Point3f(0, 0, 0), 0.0, PATH);
}