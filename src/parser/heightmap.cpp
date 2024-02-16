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
#include <core/timer.h>
#include <filesystem/resolver.h>
#include <unordered_map>
#include <fstream>
#include <ImfRgbaFile.h>
#include <ImfArray.h>

NORI_NAMESPACE_BEGIN

using namespace Imf;
using namespace Imath;

/**
 * \brief Loader for Heightmap triangle meshes
 */
class HeightMap : public Mesh {
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
    HeightMap(const PropertyList &propList) {

        filesystem::path filename = getFileResolver()->resolve(propList.getString("filename"));
        Transform trafo = propList.getTransform("toWorld", Transform());
        float min_height = propList.getFloat("minh");
        float max_height = propList.getFloat("maxh");
        float x_ratio = propList.getFloat("xratio", 1.f);
        float z_ratio = propList.getFloat("zratio", 1.f);

        Timer timer;
        std::vector<Vector3f>   positions;
        std::vector<Vector2f>   texcoords; // currently unused
        std::vector<Vector3f>   normals;
        std::vector<uint32_t>   indices;

        
        try {

            RgbaInputFile file(filename.str().c_str());
            Box2i dw = file.dataWindow();
            res_x = dw.max.x - dw.min.x + 1;
            res_z = dw.max.y - dw.min.y + 1;

            float offset_x = (res_x-1) / 2.f;
            float offset_z = (res_z-1) / 2.f;

            positions.resize(res_x * res_z);
            normals.resize(res_x * res_z);
            indices.resize(3 * 2 * (res_x - 1) * (res_z - 1));

            Array2D<Rgba> pixels(res_z, res_x);
            file.setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * res_x, 1, res_x);
            file.readPixels(dw.min.y, dw.max.y);

            // Print every pixel value
            for (int z = 0; z < res_z; ++z) {
                for (int x = 0; x < res_x; ++x) {
                    const Rgba &pixel = pixels[z][x];

                    float gray = (pixel.r + pixel.g + pixel.b) / 3.f;
                    float y = min_height + gray * (max_height - min_height);

                    float t_x = (x - offset_x) * x_ratio;
                    float t_z = (z - offset_z) * z_ratio;

                    Point3f p(t_x, y, t_z);
                    p = trafo * p;
                    m_bbox.expandBy(p);
                    positions[index(x, z)] = p; 
                }
            }

        } catch (const std::exception &e) {
            std::cerr << "Error reading EXR file: " << e.what() << std::endl;
            return;
        }

        int face_index = 0;
        for(int z = 0; z < res_z; ++z) {
            for(int x = 0; x < res_x; ++x) {
                Vector3f n = normal(x, z, positions);
                normals[index(x,z)] = n; // no need to apply trafo again

                if(x >= res_x - 1 || z >= res_z - 1) {
                    continue;
                }
                    
                int i0 = index(x, z);
                int i1 = index(x+1, z);
                int i2 = index(x+1, z+1);
                int i3 = index(x, z+1);

                float c0 = height(i0, positions);
                float c1 = height(i1, positions);
                float c2 = height(i2, positions);
                float c3 = height(i3, positions);

                // Always include the highest point in the diagonal
                if((c0 > c1 && c0 > c3) || (c2 > c1 && c2 > c3)) {
                    indices[face_index++] = i0;
                    indices[face_index++] = i1;
                    indices[face_index++] = i2;
                    indices[face_index++] = i0;
                    indices[face_index++] = i2;
                    indices[face_index++] = i3;
                } else {
                    indices[face_index++] = i1;
                    indices[face_index++] = i2;
                    indices[face_index++] = i3;
                    indices[face_index++] = i1;
                    indices[face_index++] = i3;
                    indices[face_index++] = i0;
                }
            }  
                 
        }

        m_F.resize(3, indices.size()/3);

        for (uint32_t i=0; i<indices.size()/3; ++i) {
            m_F.col(i).x() = indices[3*i];
            m_F.col(i).y() = indices[3*i+1];
            m_F.col(i).z() = indices[3*i+2];
        }

        m_V.resize(3, positions.size());
        for (uint32_t i=0; i<positions.size(); ++i)
            m_V.col(i) = positions[i];

        if (!normals.empty()) {
            m_N.resize(3, normals.size());
            for (uint32_t i=0; i<normals.size(); ++i)
                m_N.col(i) = normals[i];
        }        
        
        m_name = filename.str();
        cout << "done. (V=" << m_V.cols() << ", F=" << m_F.cols() << ", took "
             << timer.elapsedString() << " and "
             << memString(m_F.size() * sizeof(uint32_t) +
                          sizeof(float) * (m_V.size() + m_N.size() + m_UV.size()))
             << ")" << endl;
    }

protected:

    int index(int x, int z) {
        return x + res_x * z;
    }

    float height(int i, std::vector<Vector3f>& positions) {
        if(i < 0 || i >= positions.size()) {
            return 0.f;
        }
        return positions[i].y();
    }

    float height(int x, int z, std::vector<Vector3f>& positions) {
        if(x < 0 || z < 0 || x >= res_x || z >= res_z) {
            return 0.f;
        }
        return positions[index(x, z)].y();
    }

    Vector3f normal(int x, int z, std::vector<Vector3f>& positions) {
        // Interpolate normal using
        // https://stackoverflow.com/questions/13983189/opengl-how-to-calculate-normals-in-a-terrain-height-grid
        float hL = height(x - 1, z, positions);
        float hR = height(x + 1, z, positions);
        float hD = height(x, z - 1, positions);
        float hU = height(x, z + 1, positions);

        return Vector3f(hL - hR, hD - hU, 2).normalized();
    }
    /// Vertex indices used by the OBJ format
    int res_x, res_z;
};

NORI_REGISTER_CLASS(HeightMap, "hmap");
NORI_NAMESPACE_END
