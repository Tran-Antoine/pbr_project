#pragma once

#include <vector>
#include <nori/bbox.h>
#include <nori/mesh.h>
#include <sstream>
#include <nori/strutil.h>

NORI_NAMESPACE_BEGIN

template <typename T>
class Octree {

public:
    inline Octree(int depth) : depth(depth) {}
    inline ~Octree() {}

    static Octree<uint32_t>* build(BoundingBox3f env, Mesh* sceneMesh, std::vector<uint32_t>& indices=std::vector<uint32_t>(), int depth=0);
    static BoundingBox3f cut(BoundingBox3f src, int piece_index);

    inline virtual std::string pretty_print() {
        return "<pretty_print not implemented>";
    }

    inline virtual bool ray_intersects(Mesh* mesh, BoundingBox3f area, Ray3f& ray, uint32_t& index_found, float& u, float& v, float& t) {
        throw std::logic_error("Not implemented");
    }

    inline virtual uint32_t size() {
        return 0;
    }

protected:
    int depth;
};


template <typename T>
class Node : public Octree<T> {

public:

    inline Node(std::vector<Octree<T>*> children, int depth): Octree(depth), children(children) {}

    inline ~Node() {
        for(auto p : children) {
            delete p;
        }
    }

    inline bool ray_intersects(Mesh* mesh, BoundingBox3f area, Ray3f& ray, uint32_t& index_found, float& u, float& v, float& t) override {
        
        using Entry = std::pair<std::pair<float, BoundingBox3f>, Octree<T>*>;

        std::vector<Entry> distances(8);

        int child_index = 0;

        for(Octree<T>* child : children) {
            
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

    inline std::string pretty_print() override {
        
        std::ostringstream oss;
        oss << repeat(2*(depth+1), "-") + "| Node:";

        std::vector<std::string> lines;

        for(Octree<T>* c : children) {
            oss << "\n" + c->pretty_print();    
        }

        return oss.str();
    }

    inline uint32_t size() override {
        uint32_t total = 0;
        for(auto child : children) {
            total += child->size();
        }
        return total;
    }

protected:
    std::vector<Octree<T>*> children;
};

template <typename T>
class Leaf : public Octree<T> {

public:
    inline Leaf(std::vector<T> elements, int depth) : Octree(depth), elements(elements) {}

    inline bool ray_intersects(Mesh* mesh, BoundingBox3f area, Ray3f& ray, uint32_t& index_found, float& u, float& v, float& t) override {
                
        if(elements.size() == 0) return false;

        using Entry = std::pair<float, uint32_t>;

        std::vector<Entry> triangles(elements.size());
        
        int element_index = 0;

        for(T elem : elements) {
            uint32_t index = static_cast<uint32_t>(elem);
            
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

    inline std::string pretty_print() override {

        if(elements.size() == 0) {
            return repeat(2*(depth+1), "-") + "| <empty node>";
        }

        std::ostringstream oss;
        for(T e : elements) {
            oss << e << " ";
        }
        
        return repeat(2*(depth+1), "-") + "| " + oss.str();
    }

    inline uint32_t size() override {
        return elements.size();
    }


protected:
    std::vector<T> elements;
};

NORI_NAMESPACE_END