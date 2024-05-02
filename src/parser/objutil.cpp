#include <parser/objutil.h>
#include <unordered_map>
#include <fstream>
#include <core/transform.h>
#include <core/timer.h>

NORI_NAMESPACE_BEGIN

/// Vertex indices used by the OBJ format
struct OBJVertex {
    uint32_t p = (uint32_t) -1;
    uint32_t n = (uint32_t) -1;
    uint32_t uv = (uint32_t) -1;

    inline OBJVertex() {}

    inline OBJVertex(const std::string &string) {
        std::vector<std::string> tokens = tokenize(string, "/", true);

        if (tokens.size() < 1 || tokens.size() > 3)
            throw NoriException("Invalid vertex data: \"%s\"", string);

        p = toUInt(tokens[0]);

        if (tokens.size() >= 2 && !tokens[1].empty())
            uv = toUInt(tokens[1]);

        if (tokens.size() >= 3 && !tokens[2].empty())
            n = toUInt(tokens[2]);
    }

    inline bool operator==(const OBJVertex &v) const {
        return v.p == p && v.n == n && v.uv == uv;
    }
};

/// Hash function for OBJVertex
struct OBJVertexHash {
    std::size_t operator()(const OBJVertex &v) const {
        size_t hash = std::hash<uint32_t>()(v.p);
        hash = hash * 37 + std::hash<uint32_t>()(v.uv);
        hash = hash * 37 + std::hash<uint32_t>()(v.n);
        return hash;
    }
};
void load_obj_data(const std::string &filename, const Transform &trafo, std::vector<Vector3f> &positions,
                   std::vector<uint32_t> &indices, std::vector<Vector3f> &normals, std::vector<Vector2f> &texcoords, BoundingBox3f& m_box) {
    typedef std::unordered_map<OBJVertex, uint32_t, OBJVertexHash> VertexMap;

    std::ifstream is(filename);
    if (is.fail())
        throw NoriException("Unable to open OBJ file \"%s\"!", filename);

    cout << "Loading \"" << filename << "\" .. ";
    cout.flush();
    Timer timer;

    std::vector<OBJVertex> vertices;
    VertexMap vertexMap;

    std::string line_str;
    while (std::getline(is, line_str)) {
        std::istringstream line(line_str);

        std::string prefix;
        line >> prefix;

        if (prefix == "v") {
            Point3f p;
            line >> p.x() >> p.y() >> p.z();
            p = trafo * p;
            m_box.expandBy(p);
            positions.push_back(p);
        } else if (prefix == "vt") {
            Point2f tc;
            line >> tc.x() >> tc.y();
            texcoords.push_back(tc);
        } else if (prefix == "vn") {
            Normal3f n;
            line >> n.x() >> n.y() >> n.z();
            normals.push_back((trafo * n).normalized());
        } else if (prefix == "f") {
            std::string v1, v2, v3, v4;
            line >> v1 >> v2 >> v3 >> v4;
            OBJVertex verts[6];
            int nVertices = 3;

            verts[0] = OBJVertex(v1);
            verts[1] = OBJVertex(v2);
            verts[2] = OBJVertex(v3);

            if (!v4.empty()) {
                /* This is a quad, split into two triangles */
                verts[3] = OBJVertex(v4);
                verts[4] = verts[0];
                verts[5] = verts[2];
                nVertices = 6;
            }
            /* Convert to an indexed vertex list */
            for (int i = 0; i < nVertices; ++i) {
                const OBJVertex &v = verts[i];
                VertexMap::const_iterator it = vertexMap.find(v);
                if (it == vertexMap.end()) {
                    vertexMap[v] = (uint32_t) vertices.size();
                    indices.push_back((uint32_t) vertices.size());
                    vertices.push_back(v);
                } else {
                    indices.push_back(it->second);
                }
            }
        }
    }
}
NORI_NAMESPACE_END