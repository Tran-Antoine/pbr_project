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
        
        int childIndex = 0;
        bool found = false;

        for(Octree<T>* child : children) {
            
            BoundingBox3f child_area = cut(area, childIndex++);
            
            if(!child_area.rayIntersect(ray)) {
                continue;
            }

            // TODO: Intersection may not necessarily return the closest one
            // TODO: Add debug flag that stores positions?
            found |= child->ray_intersects(mesh, child_area, ray, index_found, u, v, t); 
        }

        return found;
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

protected:
    std::vector<Octree<T>*> children;
};

template <typename T>
class Leaf : public Octree<T> {

public:
    inline Leaf(std::vector<T> elements, int depth) : Octree(depth), elements(elements) {}

    inline bool ray_intersects(Mesh* mesh, BoundingBox3f area, Ray3f& ray, uint32_t& index_found, float& u, float& v, float& t) override {
        for(T elem : elements) {
            uint32_t index = static_cast<uint32_t>(elem);

            if(mesh->rayIntersect(index, ray, u, v, t)) {
                index_found = index;
                return true;
            }
        }

        return false;
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


protected:
    std::vector<T> elements;
};

NORI_NAMESPACE_END