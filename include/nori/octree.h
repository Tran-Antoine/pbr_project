#include <vector>
#include <nori/bbox.h>
#include <nori/mesh.h>

NORI_NAMESPACE_BEGIN

template <typename T>
class Octree {

public:
    Octree() {}
    ~Octree() {}

    static Octree<int>* build(BoundingBox3f* env, Mesh* sceneMesh, std::vector<int>& indices);
};

template <typename T>
class Node : public Octree<T> {

public:
    template <typename T>
    Node<T>::Node(std::vector<Octree<T>*> children): children(children) {}

    template <typename T>
    Node<T>::~Node() {
        for(auto p : children) {
            delete p;
        }
    }

protected:
    std::vector<Octree<T>*> children;
};

template <typename T>
class Leaf : public Octree<T> {

public:
    template <typename T>
    Leaf<T>::Leaf(std::vector<T> elements) : elements(elements) {}
protected:
    std::vector<T> elements;
};

NORI_NAMESPACE_END