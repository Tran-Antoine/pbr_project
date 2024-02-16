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

NORI_NAMESPACE_BEGIN

/**
 * \brief Loader for Heightmap triangle meshes
 */
class HeightMap : public Mesh {
public:

    HeightMap(const PropertyList &propList) {

        filesystem::path filename =
            getFileResolver()->resolve(propList.getString("filename"));
        
        plain = propList.getBoolean("plain", false);

        std::ifstream is(filename.str());
        if (is.fail())
            throw NoriException("Unable to open PGM file \"%s\"!", filename);
        Transform trafo = propList.getTransform("toWorld", Transform());

        cout << "Loading \"" << filename << "\" .. ";
        cout.flush();
        Timer timer;

        std::string res_str;

        // Ignore first two lines
        is.ignore(std::numeric_limits<std::streamsize>::max(), is.widen('\n'));
        is.ignore(std::numeric_limits<std::streamsize>::max(), is.widen('\n'));

        if(!std::getline(is, res_str)) {
            throw NoriException("Invalid PGM file");
        }

        std::istringstream res(res_str);
        res >> res_x >> res_z;

        if(!std::getline(is, res_str)) {
            throw NoriException("Invalid PGM file");
        }
        std::istringstream is_max(res_str);
        int max;
        is_max >> max;

        int n_positions = (res_x * res_z);
        std::cout << "Reading " << res_x << "*" << res_z << "= " << n_positions << " entries\n";

        // TODO ADD OVERFLOW CHECK
        std::vector<Vector3f>   positions(res_x * res_z);
        std::vector<Vector2f>   texcoords;
        std::vector<Vector3f>   normals(res_x * res_z);
        std::vector<uint32_t>   indices(3 * 2 * (res_x - 1) * (res_z - 1));

        float offset_x = (res_x-1) / 2.f;
        float offset_z = (res_z-1) / 2.f;

        for(int z = 0; z < res_z; ++z) {
            
            for(int x = 0; x < res_x; ++x) {

                float y = next_height(is, max);
                std::cout << "\nRead value y = " << y << "\n";
                Point3f p(x - offset_x, y, z - offset_z);

                p = trafo * p;
                m_bbox.expandBy(p);
                positions[index(x, z)] = p;             
            }

            // ignore the rest of the line, which should only be the \n char
            is.ignore(std::numeric_limits<std::streamsize>::max(), is.widen('\n'));              
        }

        int face_index = 0;
        for(int z = 0; z < res_z; ++z) {
            for(int x = 0; x < res_x; ++x) {
                Vector3f n = normal(x, z, positions);
                normals[index(x,z)] = n; // no need to apply trafo again

                if(z < res_z - 1 && x < res_x - 1) {
                    
                    int i0 = index(x, z);
                    int i1 = index(x+1, z);
                    int i2 = index(x+1, z+1);
                    int i3 = index(x, z+1);

                    float c0 = height(i0, positions);
                    float c1 = height(i1, positions);
                    float c2 = height(i2, positions);
                    float c3 = height(i3, positions);

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

        std::cout << "\n" << m_V << "\n";
        
        
        m_name = filename.str();
        cout << "done. (V=" << m_V.cols() << ", F=" << m_F.cols() << ", took "
             << timer.elapsedString() << " and "
             << memString(m_F.size() * sizeof(uint32_t) +
                          sizeof(float) * (m_V.size() + m_N.size() + m_UV.size()))
             << ")" << endl;
    }

protected:

    float next_height(std::istream& is, float max) {
        if(plain) {
            float y;
            is >> y;
            return y;

        } else {
            char msb;
            is.read(&msb, sizeof(msb));

            float y;
            if(max >= 256) {
                char lsb;
                is.read(&lsb, sizeof(lsb));
                y = (static_cast<unsigned int>(msb) << 8) | static_cast<unsigned int>(lsb);
            } else {
                y = static_cast<unsigned int>(msb);
            }
            
            return y;
        }
    }

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
        // Calculate normal using
        // https://stackoverflow.com/questions/13983189/opengl-how-to-calculate-normals-in-a-terrain-height-grid
        float hL = height(x - 1, z, positions);
        float hR = height(x + 1, z, positions);
        float hD = height(x, z - 1, positions);
        float hU = height(x, z + 1, positions);

        return Vector3f(hL - hR, hD - hU, 2).normalized();
    }
    /// Vertex indices used by the OBJ format
    int res_x, res_z;
    bool plain;
};

NORI_REGISTER_CLASS(HeightMap, "hmap");
NORI_NAMESPACE_END
