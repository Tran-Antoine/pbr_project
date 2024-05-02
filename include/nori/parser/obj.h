#pragma once

#include <core/common.h>
#include <core/mesh.h>

NORI_NAMESPACE_BEGIN

class WavefrontOBJ : public Mesh {
public:
    WavefrontOBJ(const PropertyList &propList);
    void addChild(nori::NoriObject *obj) override;
};

NORI_NAMESPACE_END