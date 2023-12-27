#include <iostream>
#include <nori/octree.h>
#include <nori/mesh.h>

int main()
{
    nori::Octree<int> test = nori::Octree<int>::build(&nori::BoundingBox3f(), nullptr);

    std::cout << "Hi" << std::endl;
    return 0;
}