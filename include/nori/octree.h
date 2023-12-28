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

    virtual std::string pretty_print() {
        return "<pretty_print function was not implemented>";
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