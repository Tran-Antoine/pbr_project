#include <nori/octree.h>
#include <iostream>
#include <numeric>
#include <algorithm>

NORI_NAMESPACE_BEGIN

template <>
static Octree<uint32_t>* Octree<uint32_t>::build(BoundingBox3f env, Mesh *sceneMesh, std::vector<uint32_t>& indices, int depth) {

    if(sceneMesh == nullptr || sceneMesh->getTriangleCount() == 0) {
        return nullptr;
    }

    uint32_t n_triangles = indices.size();

    if(n_triangles <= 10) {
        Octree<uint32_t>* tree = new Leaf<uint32_t>(indices, depth);
        return tree;
    }

    std::vector<std::vector<uint32_t>> indices_list(8);
    std::vector<BoundingBox3f> sub_boxes(8);

    for(int i = 0; i < 8; i++) {
        Point3f corner = env.getCorner(i);
        Point3f center = env.getCenter();

        Point3f min = Point3f(std::min(corner.x(), center.x()), std::min(corner.y(), center.y()), std::min(corner.z(), center.z()));
        Point3f max = Point3f(std::max(corner.x(), center.x()), std::max(corner.y(), center.y()), std::max(corner.z(), center.z()));

        sub_boxes[i] = BoundingBox3f(min, max);
    }

    for(auto triangle_index : indices) {

        for(int sub_box_index = 0; sub_box_index < 8; sub_box_index++) {
            
            BoundingBox3f sub_box_area = sub_boxes[sub_box_index];
            BoundingBox3f triangle_area = sceneMesh->getBoundingBox(triangle_index);

            if(sub_box_area.overlaps(triangle_area)) {

                indices_list[sub_box_index].push_back(triangle_index);
                break;
            }
        }
    }

    int i = 0;
    for(auto vec : indices_list) {
        std::cout << "Region " + std::to_string(i++) + " has " + std::to_string(vec.size()) + " elements" << std::endl;
    }

    std::vector<Octree<uint32_t>*> children(8);
    
    for(int sub_box_index = 0; sub_box_index < 8; sub_box_index++) {
        children[sub_box_index] = build(sub_boxes[sub_box_index], sceneMesh, indices_list[sub_box_index], depth + 1);
    }

    Node<uint32_t>* tree = new Node<uint32_t>(children, depth);

    return tree;
}

NORI_NAMESPACE_END