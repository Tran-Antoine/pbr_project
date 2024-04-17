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

#include <core/mesh.h>
#include <core/bbox.h>

NORI_NAMESPACE_BEGIN

/**
* \brief Loader for Heightmap triangle meshes
*/
class BoundedMedium : public Mesh {
public:

    /**
     * Build a mesh from a gray-scale PNG heightmap. The x-axis goes from left to right, while
     * the z-axis goes from top to bottom. The y-axis represents the height.
     * Required property values:
     *
     * * `filename`: the local path to the .png heightmap
     * * `minh`: the minimum height in meters
     * * `maxh`: the maximum height in meters
     *
     * Optional property values:
     *
     * * `xratio`: the meter/#vertices ratio in the x direction
     * * `zratio`: the meter/#vertices ratio in the z direction
     * * `toWorld`: the transformation matrix to apply
    */
    BoundedMedium(const PropertyList &propList) {}

    void build_box() {

        BoundingBox3f box = medium->bounds();
        Vector3f min = box.min, max = box.max;
        float xm = min.x(), ym = min.y(), zm = min.z(),
              xM = max.x(), yM = max.y(), zM = max.z();

        if(box.getVolume() < Epsilon) {
            throw NoriException("Invalid bounding box (too small)");
        }

        Point3f
            c000 = min,
            c001 = Point3f(xm, ym, zM),
            c010 = Point3f (xm, yM, zm),
            c011 = Point3f(xm, yM, zM),
            c100 = Point3f(xM, ym, zm),
            c101 = Point3f(xM, ym, zM),
            c110 = Point3f (xM, yM, zm),
            c111 = max;

        uint32_t
            i000 = 0,i001 = 1,i010 = 2,i011 = 3,
            i100 = 4,i101 = 5,i110 = 6,i111 = 7;

        std::vector<Vector3f> positions = {c000, c001, c010, c011, c100, c101, c110, c111};
        std::vector<uint32_t> indices = {
                i000, i001, i011,
                i000, i001, i010,

                i000, i001, i101,
                i000, i001, i100,

                i001, i011, i111,
                i001, i011, i101,

                i000, i010, i110,
                i000, i010, i100,

                i010, i011, i111,
                i010, i011, i110,

                i100, i101, i111,
                i100, i101, i110
        };

        m_F.resize(3, indices.size()/3);

        for (uint32_t i=0; i<indices.size()/3; ++i) {
            m_F.col(i).x() = indices[3*i];
            m_F.col(i).y() = indices[3*i+1];
            m_F.col(i).z() = indices[3*i+2];
        }

        m_V.resize(3, positions.size());
        for (uint32_t i = 0; i < positions.size(); ++i) {
            m_V.col(i) = positions[i];
        }
    }

    void addChild(nori::NoriObject *obj) override {
        switch (obj->getClassType()) {
            case NoriObject::EMedium:
                if(medium) {
                    throw NoriException("Cannot add multiple medium");
                }
                medium = static_cast<Medium*>(obj);
                build_box();
                break;
            default:
                Mesh::addChild(obj);
                break;
        }
    }

protected:
    Medium* medium = nullptr;
};

NORI_REGISTER_CLASS(BoundedMedium, "medium_mesh");
NORI_NAMESPACE_END
