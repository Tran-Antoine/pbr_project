#include <vector>
#include <nori/bbox.h>
#include <nori/mesh.h>
#include <sstream>
#include <nori/strutil.h>

NORI_NAMESPACE_BEGIN

template <typename T>
class Octree {

public:
    Octree(int depth) : depth(depth) {}
    ~Octree() {}

    static Octree<uint32_t>* build(BoundingBox3f env, Mesh* sceneMesh, std::vector<uint32_t>& indices, int depth=0);
    static BoundingBox3f cut(BoundingBox3f src, int piece_index);

    virtual std::string pretty_print() {
        return "<pretty_print not implemented>";
    }

    virtual bool find_intersection(Mesh* mesh, BoundingBox3f area, Ray3f& ray, uint32_t& index_found) {
        throw std::logic_error("Not implemented");
    }



protected:
    int depth;
};


template <typename T>
class Node : public Octree<T> {

public:

    Node(std::vector<Octree<T>*> children, int depth): Octree(depth), children(children) {}

    ~Node() {
        for(auto p : children) {
            delete p;
        }
    }

    bool find_intersection(Mesh* mesh, BoundingBox3f area, Ray3f& ray, uint32_t& index_found) override {
        
        int childIndex = 0;
        bool found = false;

        for(Octree<T>* child : children) {
            
            BoundingBox3f child_area = cut(area, childIndex++);
            
            if(!child_area.rayIntersect(ray)) {
                continue;
            }

            // TODO: Intersection may not necessarily return the closest one
            bool |= child->find_intersection(mesh, child_area, ray, index_found); 
        }

        return found;
    }

    std::string pretty_print() override {
        
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
    Leaf(std::vector<T> elements, int depth) : Octree(depth), elements(elements) {}

    bool find_intersection(Mesh* mesh, BoundingBox3f area, Ray3f& ray, uint32_t& index_found) override {
        for(T t : elements) {
            uint32_t index = static_cast<uint32_t>(t);
            
            BoundingBox3f triangle_box = mesh->getBoundingBox(index);

            if(triangle_box.rayIntersect(ray)) {
                index_found = index;
                return true;
            }
        }

        return false;
    }

    std::string pretty_print() override {

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