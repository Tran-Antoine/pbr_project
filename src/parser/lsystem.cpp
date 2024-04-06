#pragma once

#include <core/common.h>
#include <core/mesh.h>
#include <core/timer.h>
#include <filesystem/resolver.h>
#include <unordered_map>
#include <fstream>
#include <ImfRgbaFile.h>
#include <stack>
#include <pcg32.h>
#include <stats/warp.h>
#include <parser/lconfig.h>

NORI_NAMESPACE_BEGIN

class LSystemGrammar {

public:

    /**
     * List of symbols:
     *
     * F,G: Draw
     * [,]: Push/Restore turtle state
     * +,-: Rotate in the XY plane
     * <,>: Rotate in the XZ plane
     * s,S: Shrink/Increase width
     * l,L: Shrink/Increase length
     * r,d: Add/Remove noise feature to all values
     *
     * @param premise initial string
     * @param rules evolutionary rules
     */
    LSystemGrammar(std::string premise, std::vector<std::string> rules, pcg32& random) : premise(std::move(premise)), rules(std::move(rules)), random(random){}

    std::string evolve(int n=1) {
        std::string current = premise;
        for(int i = 0; i < n; i++) {
            for(auto rule : rules) {
                std::string temp;
                for(auto c : current) {
                    if(rule.at(0) == c) {
                        temp += rule.substr(2, rule.size());
                    } else {
                        temp += c;
                    }
                }

                current = temp;
            }
        }
        return current;
    }

private:
    std::string premise;
    std::vector<std::string> rules;
    pcg32& random;
};

struct TurtleState {

    Vector3f p;
    float yaw, pitch;
    Vector3f p_n;
    float in_thickness;
    float out_thickness;
    float length;
    bool random = false;

};

class LSystemMesh : public Mesh {

public:
    explicit LSystemMesh(const PropertyList& propList) {

        std::string premise = propList.getString("premise");
        std::vector<std::string> rules;
        for(int i = 0; i < 10; i++) {
            std::string tagName = "rule" + std::to_string(i);
            std::string rule = propList.getString(tagName, "");
            if(rule.empty()) break;

            rules.push_back(rule);
        }

        int n = propList.getInteger("evolutions");
        random.seed((uint64_t) propList.getFloat("seed", 11111));

        std::string mesh_string = LSystemGrammar(premise, rules, random).evolve(n);

        float width_factor = propList.getFloat("width_factor", 0.7f);
        float length_factor = propList.getFloat("length_factor", 0.7f);
        float pitch_term = degToRad(propList.getFloat("pitch_term", 45.f));
        float yaw_term = degToRad(propList.getFloat("yaw_term", 45.f));
        bump_increase_factor = propList.getFloat("bump_accentuate", 1.f);

        Transform trafo = propList.getTransform("toWorld", Transform());
        Timer timer;
        std::vector<Vector3f>   positions;
        std::vector<uint32_t>   indices;
        std::vector<Vector2f>   texcoords;

        std::cout << mesh_string << "\n";

        drawTree(mesh_string, width_factor, length_factor, pitch_term, yaw_term, positions, indices, texcoords);


        m_F.resize(3, indices.size()/3);

        for (uint32_t i=0; i<indices.size()/3; ++i) {
            m_F.col(i).x() = indices[3*i];
            m_F.col(i).y() = indices[3*i+1];
            m_F.col(i).z() = indices[3*i+2];
        }

        m_V.resize(3, positions.size());
        for (uint32_t i = 0; i < positions.size(); ++i) {
            m_V.col(i) = trafo * positions[i];
        }

        if (!texcoords.empty()) {
            m_UV.resize(2, positions.size());
            for (uint32_t i=0; i<texcoords.size(); ++i) {
                m_UV.col(i) = texcoords[i];
            }
        }

        cout << "done. (V=" << m_V.cols() << ", F=" << m_F.cols() << ", took "
             << timer.elapsedString() << " and "
             << memString(m_F.size() * sizeof(uint32_t) +
                          sizeof(float) * (m_V.size() + m_N.size() + m_UV.size()))
             << ")" << endl;
    }

    void addChild(nori::NoriObject *obj) override {
        DiffuseMap* map;
        switch (obj->getClassType()) {
            case NoriObject::EDiffuseMap:
                map = static_cast<DiffuseMap *>(obj);
                if(map->getId() == "normal") normal_map = map;
                else throw NoriException("Heightmap only supports normal maps");
                break;
            default:
                Mesh::addChild(obj);
                break;
        }
    }
private:

    pcg32 random;

    static void addToTextCoords(float x_texture, std::vector<Vector2f>& textCoords) {
        textCoords.push_back(Vector2f(x_texture, 0.5f));
    }

    static Vector3f directional(float pitch, float yaw) {
        float x = cos(yaw)*cos(pitch);
        float y = sin(pitch);
        float z = sin(yaw)*cos(pitch);
        return Vector3f(x, y, z);
    }

    static int idealSmoothness(float radius) {
        return 10;
        return (int) (radius * 100);
    }

    void drawTree(const std::string& seq, float width_factor, float length_factor, float pitch_term, float yaw_term,
                         std::vector<Vector3f>& positions, std::vector<uint32_t>& indices, std::vector<Vector2f>& texcoords) {

        float initial_thickness = 0.5f;
        float initial_length = 1.f;
        Config0 config = Config0(random);

        std::stack<TurtleState> turtle_states;

        TurtleState current_state = {
                Vector3f(0.f), 0.f, M_PI / 2, Vector3f(0.f, 1.f, 0.f),
                initial_thickness, initial_thickness, initial_length
        };

        TurtleState copy;

        for(char instr : seq) {
            switch(instr) {
                case '[':
                    copy = current_state;
                    turtle_states.push(copy);
                    break;
                case ']':
                    current_state = turtle_states.top();
                    turtle_states.pop();
                    break;
                case 'F':
                case 'G':
                    handleDraw(current_state, instr, positions, indices, texcoords, config);
                    break;
                case '+':
                    current_state.pitch += pitch_term;
                    break;
                case '-':
                    current_state.pitch -= pitch_term;
                    break;
                case '>':
                    current_state.yaw += yaw_term;
                    break;
                case '<':
                    current_state.yaw -= yaw_term;
                    break;
                case 'w':
                    current_state.out_thickness *= width_factor;
                    break;
                case 'W':
                    current_state.out_thickness *= 1 / width_factor;
                    break;
                case 'l':
                    current_state.length *= length_factor;
                    break;
                case 'L':
                    current_state.length *= 1 / length_factor;
                    break;
                case 'r':
                    current_state.random = true;
                    break;
                case 'd':
                    current_state.random = false;
                default: break;
            }
        }
    }

    void handleDraw(TurtleState &state, char c, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f>& texcoords,
                    LGrammarConfig& config) {

        Vector3f a = state.p;
        Vector3f a_n = state.p_n;

        bool randomization = state.random;

        float length = state.length;
        float yaw = state.yaw, pitch = state.pitch;
        float in_thickness = state.in_thickness;
        float out_thickness = state.out_thickness;

        if(randomization) {
            length = config.randomizeLength(length, c);
            yaw = config.randomizeYaw(yaw, c);
            pitch = config.randomizePitch(pitch, c);
            out_thickness = config.randomizeThickness(out_thickness, c);
        }

        Vector3f direction = directional(pitch, yaw);
        Vector3f b = a + length * direction;
        Vector3f b_n = (b-a).normalized();

        connect(a, b, a_n, in_thickness, out_thickness, idealSmoothness(std::max(in_thickness, out_thickness)),
                positions, indices, texcoords);
        state.p = b;
        state.p_n = b_n;
        state.in_thickness = out_thickness;
    }

    static void rotateXY(Vector3f& current, float angle) {
        float x = current.x(), y = current.y(), z = current.z();
        float cosTheta = cos(angle), sinTheta = sin(angle);

        current = Vector3f(
                    x*cosTheta - y*sinTheta,
                    x*sinTheta + y*cosTheta,
                    z
                );
    }

    static void rotateXZ(Vector3f& current, float angle) {
        float x = current.x(), y = current.y(), z = current.z();
        float cosTheta = cos(angle), sinTheta = sin(angle);

        current = Vector3f(
                x*cosTheta - z*sinTheta,
                y,
                x*sinTheta + z*cosTheta
        );
    }

    static void
    connect(const Point3f &a, const Point3f &b, const Vector3f &a_n, float in_thickness, float out_thickness,
            int smoothness, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices,
            std::vector<Vector2f> &texcoords) {

        Vector3f b_n = (b-a).normalized(); // normal of b must be according to the cylinder's direction

        std::vector<Vector3f> from_circle = circle(a, a_n, in_thickness, smoothness);
        std::vector<Vector3f> to_circle   = circle(b, b_n, out_thickness, smoothness);

        int index_pointer = positions.size(); // save pointer before adding new vertices
        push_circle(from_circle, a, positions, indices, texcoords, false);
        push_circle(to_circle, b, positions, indices, texcoords, true);

        int n_points = from_circle.size() + 1; // +1 due to the center point

        // goal: form two triangles to shape the rectangle of points (a0, a1, b0, b1)
        int i0 = index_pointer + 1;
        int j0 = i0 + n_points;

        // For now we only allow smoothness to control the number of points, not the "length" resolution
        for(int i = 1; i < n_points - 1; i++) {

            int i1 = i0 + 1;
            int j1 = j0 + 1;

            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(j1);

            indices.push_back(i0);
            indices.push_back(j1);
            indices.push_back(j0);

            i0++;
            j0++;
        }

        indices.push_back(i0);
        indices.push_back(index_pointer + 1);
        indices.push_back(index_pointer + 1 + n_points);

        indices.push_back(i0);
        indices.push_back(index_pointer + 1 + n_points);
        indices.push_back(j0);
    }

    static std::vector<Vector3f> circle(const Vector3f& p, const Vector3f& p_n, float radius, int smoothness) {

        std::vector<Vector3f> pos(4 + smoothness);
        Vector3f test = Vector3f(-p_n.y(), p_n.x(), 0).normalized();
        Vector3f test2 = p_n.cross(test).normalized();
        Frame frame(test, test2, p_n);
        Vector3f anchor = Vector3f(radius, 0, 0);

        for(int j = 0; j < smoothness + 4; j++) {

            float delta_angle = j * 2 * M_PI * (1.f / (4 + smoothness));
            //    x' = x cos θ − y sin θ
            //    y' = x sin θ + y cos θ
            Vector3f rotated_point = Vector3f(anchor.x() * cos(delta_angle) - anchor.y() * sin(delta_angle),
                                              anchor.x() * sin(delta_angle) + anchor.y() * cos(delta_angle),
                                              0.f);

            pos[j] = frame.toWorld(rotated_point) + p;
        }

        return (pos);
    }

    static void push_circle(std::vector<Vector3f> &circle_positions, const Vector3f &p, std::vector<Vector3f> &positions,
                std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords,
                bool fill_circle) {

        int center_pointer = positions.size();
        int head = positions.size() + 1;
        int edge_pointer = head;

        float y_texture = fill_circle ? 1.f : 0.f;

        positions.push_back(p);
        texcoords.push_back(Vector2f(0.5f, y_texture)); // the value of this one doesn't really matter

        Vector3f edge0 = circle_positions[0];
        positions.push_back(edge0);
        texcoords.push_back(Vector2f(0.0f, y_texture));

        for(int i = 1; i <= circle_positions.size() - 1; i++) {

            float x_texture = (float) i / (float) circle_positions.size();

            Vector3f edge1 = circle_positions[i];
            positions.push_back(edge1);
            texcoords.push_back(Vector2f(x_texture, y_texture));


            if(fill_circle) {
                indices.push_back(center_pointer);
                indices.push_back(edge_pointer);
                indices.push_back(edge_pointer+1);
            }

            edge_pointer++;
        }

        if(fill_circle) {
            indices.push_back(center_pointer);
            indices.push_back(edge_pointer);
            indices.push_back(head);
        }
    }
};

NORI_REGISTER_CLASS(LSystemMesh, "lsystem");
NORI_NAMESPACE_END