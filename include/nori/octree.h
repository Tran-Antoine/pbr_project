#pragma once

#include <vector>
#include <nori/bbox.h>
#include <nori/mesh.h>
#include <sstream>
#include <nori/strutil.h>

NORI_NAMESPACE_BEGIN

/**
 * @brief Optimized data-structure for ray intersection through a large set of triangles.
*/
class Octree {

public:
    
    /// Deconstructs the Octree. The implementation of the superclass does nothing.
    ~Octree();

    /// @brief Constructs an Octree storing uint32_t values from scene data
    /// @param env the bounding box surrounding the whole scene
    /// @param sceneMesh the mesh of the scene
    /// @param indices which triangles are to be considered in the node
    /// @param depth the depth of the node
    /// @return an Octree (most likely a Node object)
    static Octree* build(BoundingBox3f env, Mesh* sceneMesh, std::vector<uint32_t>& indices=std::vector<uint32_t>(), int depth=0);
    
    /// @brief Cuts a bounding box into 8 regions of equal volume and returns one of the regions
    /// @param src the bounding box to cut
    /// @param piece_index which region to return
    /// @return a bounding box representing one eigth of the provided box
    static BoundingBox3f cut(BoundingBox3f src, int piece_index);

    /// @brief 
    /// @return a string representation of the tree
    virtual std::string pretty_print();

    /// @brief Computes ray intersection.
    /// @param mesh the mesh of the scene
    /// @param area the bounding box surrounding the scene, needs to be the same as the one used to construct the octree
    /// @param ray the ray to be tested for intersection
    /// @param index_found upon success, contains the index of the triangle found to intersect with the ray
    /// @param u upon success, contains the u vector of the triangle
    /// @param v upon success, contains the v vector of the triangle
    /// @param t upon success, contains the distance t from the ray origin to the intersection point
    /// @return whether an intersection was found
    virtual bool ray_intersects(Mesh* mesh, BoundingBox3f area, Ray3f& ray, uint32_t& index_found, float& u, float& v, float& t);
    
    /// @brief 
    /// @return the number of values stored in the tree 
    virtual uint32_t size();

protected:
    
    /// Constructs an Octree with a given depth.
    Octree(int depth);

    /// @brief The depth of the node. Root is depth=0
    int depth;
};


class Node : public Octree {

public:

    Node(std::vector<Octree*> children, int depth);
    ~Node();
    bool ray_intersects(Mesh* mesh, BoundingBox3f area, Ray3f& ray, uint32_t& index_found, float& u, float& v, float& t) override;
    std::string pretty_print() override;
    uint32_t size() override;

protected:
    std::vector<Octree*> children;
};

class Leaf : public Octree {

public:
    Leaf(std::vector<uint32_t> elements, int depth);
    bool ray_intersects(Mesh* mesh, BoundingBox3f area, Ray3f& ray, uint32_t& index_found, float& u, float& v, float& t) override;
    std::string pretty_print() override;
    uint32_t size() override;

protected:
    std::vector<uint32_t> elements;
};

NORI_NAMESPACE_END