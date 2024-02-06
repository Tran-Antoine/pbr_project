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

NORI_NAMESPACE_BEGIN

struct EmitterQueryRecord {

    bool empty;
    Point3f p, l;
    Vector3f n_p, n_l, wi;
    const BSDF* bsdf;

    EmitterQueryRecord() : empty(true) {}

    EmitterQueryRecord(const BSDF* bsdf, Point3f p, Vector3f n_p, Vector3f wi) : bsdf(bsdf), p(p), n_p(n_p), wi(wi), empty(false) {}

    EmitterQueryRecord(const BSDF* bsdf, Point3f p, Vector3f n_p, Vector3f wi, Point3f l, Vector3f n_l) : bsdf(bsdf), p(p), n_p(n_p), l(l), n_l(n_l), wi(wi), empty(false) {}

    bool isEmpty() const { return empty; }

    Vector3f wo() const {
        return (l-p).normalized();
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

    virtual Color3f evalRadiance(EmitterQueryRecord& rec, const Scene* scene) const = 0;

    virtual Color3f sampleRadiance(EmitterQueryRecord& rec, Sampler& sampler, const Scene* scene, float& angular_pdf) const = 0;

    virtual Color3f getEmittance(Point3f pos, Vector3f normal, Vector3f direction) const = 0;
};



NORI_NAMESPACE_END