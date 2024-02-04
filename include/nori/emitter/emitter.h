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

    virtual Color3f sampleRadiance(const BSDF* bsdf, Point3f at, Vector3f at_normal, Vector3f dir, Sampler& sampler, const Scene* scene) const = 0;

    virtual Color3f getEmittance(Point3f pos, Vector3f normal, Vector3f direction) const = 0;
};



NORI_NAMESPACE_END
