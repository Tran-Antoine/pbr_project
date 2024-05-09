#pragma once

#include <core/common.h>
#include <core/color.h>
#include <core/vector.h>
#include <core/object.h>

#include <utility>

NORI_NAMESPACE_BEGIN

class DiffuseMap : public NoriObject {

public:

    explicit DiffuseMap(std::string  id) : id(std::move(id)) {}
    explicit DiffuseMap(const PropertyList& propList) : id(propList.getString("id", "no-id")) {}

    virtual Color3f T(float s, float t) = 0;

    inline Color3f T(Point2f p) {
        return T(p.x(), p.y());
    }

    EClassType getClassType() const override { return EDiffuseMap; }

    std::string getId() const { return id; }

private:
    std::string id;
};

NORI_NAMESPACE_END