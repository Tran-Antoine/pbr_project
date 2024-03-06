/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <core/object.h>
#include <stats/sampler.h>
#include <core/scene.h>
#include <bsdf/bsdf.h>
#include <core/ray.h>

#include <utility>

NORI_NAMESPACE_BEGIN

struct EmitterQueryRecord {

    bool empty;
    Point3f p, l;
    Vector3f n_p, n_l, wi;
    Point2f uv;
    const BSDF* bsdf;

    EmitterQueryRecord() : empty(true) {}

    EmitterQueryRecord(const BSDF* bsdf, Point3f p, Vector3f n_p, Vector3f wi, Point2f uv) :
        bsdf(bsdf), p(std::move(p)), n_p(std::move(n_p)), wi(std::move(wi)), uv(std::move(uv)), empty(false) {}

    EmitterQueryRecord(const BSDF* bsdf, Point3f p, Vector3f n_p, Vector3f wi, Point3f l, Vector3f n_l, Point2f uv) :
        bsdf(bsdf), p(std::move(p)), n_p(std::move(n_p)), l(std::move(l)), n_l(std::move(n_l)), wi(std::move(wi)), uv(std::move(uv)), empty(false) {}

    bool isEmpty() const { return empty; }

    Vector3f wo() const {
        return (l-p).normalized();
    }

    float dist() const {
        return (l-p).norm();
    }
};

/**
 * \brief Superclass of all emitters
 */
class Emitter : public NoriObject {
public:

    /**
     * \brief Return the type of object (i.e. Mesh/Emitter/etc.) 
     * provided by this instance
     * */
    EClassType getClassType() const { return EEmitter; }

    virtual float pdf(const EmitterQueryRecord& rec) const = 0;

    virtual Color3f evalRadiance(const EmitterQueryRecord &rec, const Scene* scene) const = 0;

    virtual Color3f sampleRadiance(EmitterQueryRecord& rec, Sampler& sampler, const Scene* scene, float& angular_pdf) const = 0;

    virtual Color3f getEmittance(const EmitterQueryRecord &rec) const = 0;

protected:

    virtual void samplePoint(Sampler &sampler, EmitterQueryRecord &rec, float &pdf) const = 0;

    Color3f evalBSDF(const EmitterQueryRecord& rec) const;
    bool is_source_visible(const Scene* scene, const EmitterQueryRecord& rec) const;
    float angular_distortion(const EmitterQueryRecord& rec) const;
    float to_angular(const EmitterQueryRecord& rec, float pdf) const;
};


NORI_NAMESPACE_END
