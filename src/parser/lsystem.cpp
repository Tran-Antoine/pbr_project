#pragma once

#include <core/common.h>
#include <core/mesh.h>
#include <core/timer.h>
#include <filesystem/resolver.h>
#include <unordered_map>
#include <fstream>
#include <ImfRgbaFile.h>
#include <stack>

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
     *
     *
     * @param premise initial string
     * @param rules evolutionary rules
     */
    LSystemGrammar(std::string premise, std::vector<std::string> rules) : premise(std::move(premise)), rules(std::move(rules)){}

    std::string evolve(int n=1) {
        std::string current = premise;
        for(int i = 0; i < n; i++) {
            std::string temp;
            for(auto rule : rules) {
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
};

struct TurtleState {

    Vector3f p;
    float yaw, pitch;
    Vector3f p_n;
    float in_thickness;
    float out_thickness;
    float length;

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

        std::string mesh_string = LSystemGrammar(premise, rules).evolve(n);

        Transform trafo = propList.getTransform("toWorld", Transform());
        Timer timer;
        std::vector<Vector3f>   positions;
        std::vector<uint32_t>   indices;

        std::cout << mesh_string << "\n";
        drawTree(mesh_string, positions, indices);
        Vector3f a = Vector3f(0, 0, 0);
        Vector3f a_n = Vector3f(-0.7f, 0.7f, 0.f).normalized();
        Vector3f b = Vector3f(-2.f, 1.9f, 0.f);

        //connect(a, b, a_n, 0.5f, 0.5f, 0, positions, indices);

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

        cout << "done. (V=" << m_V.cols() << ", F=" << m_F.cols() << ", took "
             << timer.elapsedString() << " and "
             << memString(m_F.size() * sizeof(uint32_t) +
                          sizeof(float) * (m_V.size() + m_N.size() + m_UV.size()))
             << ")" << endl;
    }
private:

    static Vector3f directional(const TurtleState& state) {
        float x = cos(state.yaw)*cos(state.pitch);
        float y = sin(state.pitch);
        float z = sin(state.yaw)*cos(state.pitch);
        return Vector3f(x, y, z);
    }

    static void drawTree(const std::string& seq,
                         std::vector<Vector3f>& positions, std::vector<uint32_t>& indices) {

        float thickness = 0.001f;
        float length = 0.3f;
        int smoothness = 5;
        float angle = M_PI / 6;


        std::stack<TurtleState> turtle_states;

        TurtleState current_state = {
                Vector3f(0.f), 0.f, M_PI / 2, Vector3f(0.f, 1.f, 0.f),
                thickness, thickness, length
        };

        TurtleState copy;
        Vector3f a, b, a_n, b_n; // C++ being a pain in the butt

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
                    a = current_state.p;
                    b = a + current_state.length * directional(current_state);

                    a_n = current_state.p_n;
                    b_n = (b-a).normalized();
                    connect(a, b, a_n, current_state.in_thickness, current_state.out_thickness, smoothness,
                            positions, indices);
                    current_state.p = b;
                    current_state.p_n = b_n;
                    current_state.in_thickness = current_state.out_thickness;
                    break;
                case '+':
                    current_state.pitch += angle;
                    break;
                case '-':
                    current_state.pitch -= angle;
                    break;
                case '>':
                    current_state.yaw += angle;
                    break;
                case '<':
                    current_state.yaw -= angle;
                    break;
                case 's':
                    current_state.out_thickness *= 0.7f;
                    break;
                case 'S':
                    current_state.out_thickness *= 1/0.7f;
                    break;
                case 'l':
                    current_state.length *= 0.7f;
                    break;
                case 'L':
                    current_state.length *= 1/0.7f;
                    break;
                default: break;
            }
        }
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

    static void connect(const Point3f& a, const Point3f& b, const Vector3f& a_n, float in_thickness, float out_thickness, int smoothness,
                        std::vector<Vector3f>& positions, std::vector<uint32_t>& indices) {

        Vector3f b_n = (b-a).normalized(); // normal of b must be according to the cylinder's direction

        std::vector<Vector3f> from_circle = circle(a, a_n, in_thickness, smoothness);
        std::vector<Vector3f> to_circle   = circle(b, b_n, out_thickness, smoothness);

        int index_pointer = positions.size(); // save pointer before adding new vertices
        push_circle(from_circle, a, a_n, positions, indices, false);
        push_circle(to_circle,   b, b_n, positions, indices, false);

        int n_points = from_circle.size() + 1; // +1 due to the center point

        // goal: form two triangles to shape the rectangle of points (a0, a1, b0, b1)
        int i0 = index_pointer + 1;
        int j0 = i0 + n_points;

        // For now we only allow smoothness to control the number of points, not the "length" resolution
        for(int i = 1; i < n_points - 1; i++) {

            int i1 = i0 + 1;
            int j1 = j0 + 1;

            // TODO: check if faces are provided in the right order
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(j1);

            indices.push_back(i0);
            indices.push_back(j1);
            indices.push_back(j0);

            i0++;
            j0++;

            //std::cout << "Face 1: " << i0 << "," << i1 << "," << j1 << "\n";
            //std::cout << "Face 2: " << i0 << "," << j1 << "," << j0 << "\n";

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
        Frame frame(p_n);
        Vector3f anchor = Vector3f(radius, 0, 0);

        for(int j = 0; j < smoothness + 4; j++) {

            float delta_angle = j * 2 * M_PI * (1.f / (4 + smoothness));
            //    x' = x cos θ − y sin θ
            //    y' = x sin θ + y cos θ
            Vector3f rotated_point = Vector3f(anchor.x() * cos(delta_angle) - anchor.y() * sin(delta_angle),
                                              anchor.x() * sin(delta_angle) - anchor.y() * cos(delta_angle),
                                              0.f);

            // TODO: check if this is correct
            pos[j] = frame.toWorld(rotated_point) + p;
        }

        return (pos);
    }

    static void push_circle(std::vector<Vector3f>& circle_positions, const Vector3f& p, const Vector3f& p_n,
                            std::vector<Vector3f>& positions, std::vector<uint32_t>& indices, bool fill_circle) {

        int center_pointer = positions.size();
        int head = positions.size() + 1;
        int edge_pointer = head;

        positions.push_back(p);

        Vector3f edge0 = circle_positions[0];
        positions.push_back(edge0);

        for(int i = 1; i <= circle_positions.size() - 1; i++) {

            Vector3f edge1 = circle_positions[i];
            positions.push_back(edge1);

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