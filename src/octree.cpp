#include <nori/octree.h>
#include <iostream>
#include <numeric>
#include <algorithm>

NORI_NAMESPACE_BEGIN


Octree::Octree(int depth) : depth(depth) {}
Octree::~Octree() {}

std::string Octree::pretty_print() {
    return "<pretty_print not implemented>";
}

bool Octree::ray_intersects(Mesh* mesh, BoundingBox3f area, Ray3f& ray, uint32_t& index_found, float& u, float& v, float& t) {
    throw NoriException("Not implemented");
}

uint32_t Octree::size() {
    throw NoriException("Not implemented");
}

static bool process_triangle(uint32_t index, std::vector<std::vector<uint32_t>>& indices_list, std::vector<BoundingBox3f>& sub_boxes, Mesh* sceneMesh) {
    for(int sub_box_index = 0; sub_box_index < 8; sub_box_index++) {
            
        BoundingBox3f sub_box_area = sub_boxes[sub_box_index];
        BoundingBox3f triangle_area = sceneMesh->getBoundingBox(index);

        if(sub_box_area.overlaps(triangle_area)) {

            indices_list[sub_box_index].push_back(index);
        }
    }
    return false;
}

Octree* Octree::build(BoundingBox3f env, Mesh *sceneMesh, std::vector<uint32_t>& indices, int depth) {

    if(sceneMesh == nullptr || sceneMesh->getTriangleCount() == 0) {
        return nullptr;
    }

    uint32_t n_triangles = depth == 0 ? sceneMesh->getTriangleCount() : indices.size();

    // for depth above 15 the trade-off just isn't worth it (and may crash)
    // so far 8 seems to be a good threshold.
    if(n_triangles <= 10 || depth >= 8) { 
        Octree* tree = new Leaf(indices, depth);
        return tree;
    }

    std::vector<std::vector<uint32_t>> indices_list(8);
    std::vector<BoundingBox3f> sub_boxes(8);

    for(int i = 0; i < 8; i++) {
        sub_boxes[i] = cut(env, i);
    }

    if(depth == 0 && indices.size() == 0) { // meaning we need to go through all indices
        for(uint32_t triangle_index = 0; triangle_index < n_triangles; triangle_index++) {
            process_triangle(triangle_index, indices_list, sub_boxes, sceneMesh);
        }
    }
    else {
        for(auto triangle_index : indices) {
            process_triangle(triangle_index, indices_list, sub_boxes, sceneMesh);
        }
    }

    std::vector<Octree*> children(8);
    
    for(int sub_box_index = 0; sub_box_index < 8; sub_box_index++) {
        children[sub_box_index] = build(sub_boxes[sub_box_index], sceneMesh, indices_list[sub_box_index], depth + 1);
    }

    Node* tree = new Node(children, depth);

    return tree;
}

BoundingBox3f Octree::cut(BoundingBox3f src, int piece_index) {
    Point3f corner = src.getCorner(piece_index);
    Point3f center = src.getCenter();

    Point3f min = Point3f(std::min(corner.x(), center.x()), std::min(corner.y(), center.y()), std::min(corner.z(), center.z()));
    Point3f max = Point3f(std::max(corner.x(), center.x()), std::max(corner.y(), center.y()), std::max(corner.z(), center.z()));

    return BoundingBox3f(min, max);
}

Node::Node(std::vector<Octree*> children, int depth): Octree(depth), children(children) {}
Node::~Node() {
    for(auto p : children) {
        delete p;
    }
}

bool Node::ray_intersects(Mesh* mesh, BoundingBox3f area, Ray3f& ray, uint32_t& index_found, float& u, float& v, float& t) {
        
    using Entry = std::pair<std::pair<float, BoundingBox3f>, Octree*>;

    std::vector<Entry> distances(8);

    int child_index = 0;

    for(Octree* child : children) {
        
        BoundingBox3f child_area = cut(area, child_index);
        float child_distance, out;

        if(child_area.rayIntersect(ray, child_distance, out)) {
            distances[child_index] = {{child_distance, child_area}, child};
        } else {
            distances[child_index] = {{std::numeric_limits<float>::infinity(), child_area}, nullptr};
        }

        child_index++;
    }

    std::sort(distances.begin(), distances.end(),
            [](const Entry& a, const Entry& b) {
                return a.first.first < b.first.first;
            });

    for(Entry entry : distances) {
        if(entry.second == nullptr) {
            return false;
        }

        BoundingBox3f child_area = entry.first.second;

        if(entry.second->ray_intersects(mesh, child_area, ray, index_found, u, v, t)) {
            return true;
        }
    }

    return false;
}

std::string Node::pretty_print() {
        
    std::ostringstream oss;
    oss << repeat(2*(depth+1), "-") + "| Node:";

    std::vector<std::string> lines;

    for(Octree* c : children) {
        oss << "\n" + c->pretty_print();    
    }

    return oss.str();
}

uint32_t Node::size() {
    uint32_t total = 0;
    for(auto child : children) {
        total += child->size();
    }
    return total;
}

Leaf::Leaf(std::vector<uint32_t> elements, int depth) : Octree(depth), elements(elements) {}

bool Leaf::ray_intersects(Mesh* mesh, BoundingBox3f area, Ray3f& ray, uint32_t& index_found, float& u, float& v, float& t) {
                
    if(elements.size() == 0) return false;

    using Entry = std::pair<float, uint32_t>;

    std::vector<Entry> triangles(elements.size());
    
    int element_index = 0;

    for(uint32_t index : elements) {
        
        float min_distance;
        float temp_u, temp_v; // unused


        if(mesh->rayIntersect(index, ray, temp_u, temp_v, min_distance)) {
            triangles[element_index] = {min_distance, index};
        } else {
            triangles[element_index] = {std::numeric_limits<float>::infinity(), index};
        }

        element_index++;
    }

    std::sort(triangles.begin(), triangles.end(),
            [](const Entry& a, const Entry& b) {
                return a.first < b.first;
            });

    uint32_t closestIndex = triangles[0].second;
    index_found = closestIndex;
    
    return mesh->rayIntersect(closestIndex, ray, u, v, t);
}

std::string Leaf::pretty_print() {

    if(elements.size() == 0) {
        return repeat(2*(depth+1), "-") + "| <empty node>";
    }

    std::ostringstream oss;
    for(uint32_t e : elements) {
        oss << e << " ";
    }
    
    return repeat(2*(depth+1), "-") + "| " + oss.str();
}

uint32_t Leaf::size() {
    return elements.size();
}

NORI_NAMESPACE_END