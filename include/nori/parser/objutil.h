#pragma once

#include <core/common.h>
#include <core/transform.h>
#include <core/bbox.h>

NORI_NAMESPACE_BEGIN

void load_obj_data(const std::string &filename, const Transform &trafo, std::vector<Vector3f> &positions,
                   std::vector<uint32_t> &indices, std::vector<Vector3f> &normals, std::vector<Vector2f> &texcoords, BoundingBox3f& m_box);

NORI_NAMESPACE_END