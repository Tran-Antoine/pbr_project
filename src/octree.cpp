#include <nori/octree.h>
#include <iostream>
#include <numeric>
#include <algorithm>

NORI_NAMESPACE_BEGIN

template <>
static Octree<int>* Octree<int>::build(BoundingBox3f *env, Mesh *sceneMesh, std::vector<int>& indices) {

    if(sceneMesh == nullptr || sceneMesh->getTriangleCount() == 0) {
        return nullptr;
    }

    uint32_t n_triangles = sceneMesh->getTriangleCount();

    if(n_triangles <= 10) {
        Octree<int>* tree = new Leaf<int>(indices);
        return tree;
    }

    std::vector<std::vector<int>> indices_list(8);
    std::vector<BoundingBox3f*> sub_boxes(8);

    for(int i = 0; i < 8; i++) {
            Point3f corner = env->getCorner(i);
            Point3f center = env->getCenter();

            Point3f min = Point3f(std::min(corner.x(), center.x()), std::min(corner.y(), center.y()), std::min(corner.z(), center.z()));
            Point3f max = Point3f(std::max(corner.x(), center.x()), std::max(corner.y(), center.y()), std::max(corner.z(), center.z()));

            sub_boxes[i] = new BoundingBox3f(min, max);
    }

    for(uint32_t triangle_index = 0; triangle_index < n_triangles; ++triangle_index) {
        for(int sub_box_index = 0; sub_box_index < 8; sub_box_index++) {
            
            
            BoundingBox3f* sub_box_area = sub_boxes[sub_box_index];
            BoundingBox3f triangle_area = sceneMesh->getBoundingBox(triangle_index);

            if(sub_box_area->overlaps(triangle_area)) {
                indices_list[sub_box_index].push_back(triangle_index);
                break;
            }
        }
    }

    std::vector<Octree<int>*> children(8);
    
    for(int sub_box_index = 0; sub_box_index < 8; sub_box_index++) {
        children[sub_box_index] = build(sub_boxes[sub_box_index], sceneMesh, indices_list[0]);
    }

    Node<int>* tree = new Node<int>(children);

    return tree;
}

NORI_NAMESPACE_END