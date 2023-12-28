#include <nori/octree.h>
#include <iostream>
#include <numeric>
#include <algorithm>

NORI_NAMESPACE_BEGIN

static bool process_triangle(uint32_t index, std::vector<std::vector<uint32_t>>& indices_list, std::vector<BoundingBox3f>& sub_boxes, Mesh* sceneMesh) {
    for(int sub_box_index = 0; sub_box_index < 8; sub_box_index++) {
            
            BoundingBox3f sub_box_area = sub_boxes[sub_box_index];
            BoundingBox3f triangle_area = sceneMesh->getBoundingBox(index);

            if(sub_box_area.overlaps(triangle_area)) {

                indices_list[sub_box_index].push_back(index);
                return true;
            }
    }
    return false;
}

template <>
static Octree<uint32_t>* Octree<uint32_t>::build(BoundingBox3f env, Mesh *sceneMesh, std::vector<uint32_t>& indices, int depth) {

    if(sceneMesh == nullptr || sceneMesh->getTriangleCount() == 0) {
        return nullptr;
    }

    uint32_t n_triangles = depth == 0 ? sceneMesh->getTriangleCount() : indices.size();

    if(n_triangles <= 10) {
        Octree<uint32_t>* tree = new Leaf<uint32_t>(indices, depth);
        return tree;
    }

    std::vector<std::vector<uint32_t>> indices_list(8);
    std::vector<BoundingBox3f> sub_boxes(8);

    for(int i = 0; i < 8; i++) {
        sub_boxes[i] = cut(env, i);
    }

    if(depth == 0 && indices.size() == 0) {
        for(uint32_t triangle_index = 0; triangle_index < n_triangles; triangle_index++) {
            if(process_triangle(triangle_index, indices_list, sub_boxes, sceneMesh)) continue;
        }
    }
    else {
        for(auto triangle_index : indices) {
            if(process_triangle(triangle_index, indices_list, sub_boxes, sceneMesh)) continue;
        }
    }

    std::vector<Octree<uint32_t>*> children(8);
    
    for(int sub_box_index = 0; sub_box_index < 8; sub_box_index++) {
        children[sub_box_index] = build(sub_boxes[sub_box_index], sceneMesh, indices_list[sub_box_index], depth + 1);
    }

    Node<uint32_t>* tree = new Node<uint32_t>(children, depth);

    return tree;
}

template <>
static BoundingBox3f Octree<uint32_t>::cut(BoundingBox3f src, int piece_index) {
    Point3f corner = src.getCorner(piece_index);
    Point3f center = src.getCenter();

    Point3f min = Point3f(std::min(corner.x(), center.x()), std::min(corner.y(), center.y()), std::min(corner.z(), center.z()));
    Point3f max = Point3f(std::max(corner.x(), center.x()), std::max(corner.y(), center.y()), std::max(corner.z(), center.z()));

    return BoundingBox3f(min, max);
}

NORI_NAMESPACE_END