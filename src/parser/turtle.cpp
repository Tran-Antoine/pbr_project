#pragma once

#include <parser/turtle.h>
#include <parser/objutil.h>
#include <parser/obj.h>
#include <stats/warp.h>

NORI_NAMESPACE_BEGIN

int idealSmoothness(float radius) {
    return (int) (radius * 100);
}

void drawMesh(const std::string &filename, const Transform &trafo, std::vector<Vector3f> &positions,
              std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) {


    int starting_index = positions.size();

    auto props = PropertyList();
    props.setString("filename", filename);
    props.setTransform("toWorld", trafo);
    props.setBoolean("no-print", true);

    auto mesh = WavefrontOBJ(props);

    auto positions_ = mesh.getVertexPositions();
    for(int i = 0; i < positions_.cols(); i++) {
        positions.push_back(positions_.col(i));
    }

    auto indices_ = mesh.getIndices();
    for(int i = 0; i < indices_.cols(); i++) {
        auto column = indices_.col(i);
        indices.push_back(starting_index + column.x());
        indices.push_back(starting_index + column.y());
        indices.push_back(starting_index + column.z());
    }

    auto texcoords_ = mesh.getVertexTexCoords();
    for(int i = 0; i < texcoords_.cols(); i++) {
        texcoords.push_back(texcoords_.col(i));
    }
}

void drawCylinder(float length, float yaw, float pitch, float in_thickness, float out_thickness,
                         std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f>& texcoords, TurtleState &state) {

    Vector3f a = state.p;
    Vector3f a_n = state.p_n;
    Vector3f direction = directional(pitch, yaw);
    Vector3f b = a + length * direction;
    Vector3f b_n = (b-a).normalized();

    if(abs(a_n.dot(b_n)) < 0.5) {
        // start with the out thickness right away
        drawStraightCylinder(a, b, out_thickness, out_thickness, positions, indices, texcoords, state);
    }

    connect(a, b, a_n, in_thickness, out_thickness, idealSmoothness(std::max(in_thickness, out_thickness)),
            positions, indices, texcoords);
    state.p = b;
    state.p_n = b_n;
    state.in_thickness = out_thickness;
}

void drawCylinder(const Point3f& a, const Point3f& b, const Vector3f& a_n, const Vector3f& b_n, float in_thickness, float out_thickness,
                  std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f>& texcoords, TurtleState &state) {

    connect(a, b, a_n, in_thickness, out_thickness, idealSmoothness(std::max(in_thickness, out_thickness)),
            positions, indices, texcoords);
    state.p = b;
    state.p_n = b_n;
    state.in_thickness = out_thickness;
}

static void addNoise(float& yaw, float& pitch, float sample, float variance) {
    yaw *= 1 + Warp::lineToLogistic(sample, variance);
    pitch *= 1 + Warp::lineToLogistic(sample, variance);
}

void drawCylinder(float length, float yaw, float pitch, float in_thickness, float out_thickness, int n_nodes, float variance, float sample,
                  std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f>& texcoords, TurtleState &state) {
    float dl = length / (float) n_nodes;
    float dt = (out_thickness - in_thickness) / (float) n_nodes;

    Vector3f end_point = state.p + length * directional(pitch, yaw);
    Vector3f next;

    state.in_thickness = in_thickness;

    for(int i = 0; i < n_nodes - 1; i++) {

        addNoise(yaw, pitch, sample, variance);
        next = state.p + dl * directional(pitch, yaw);
        drawCylinder(dl, yaw, pitch, state.in_thickness, state.in_thickness + dt, positions, indices, texcoords, state);
    }

    drawCylinder(state.p, end_point, state.p_n, (end_point - next).normalized(), state.in_thickness, out_thickness, positions, indices, texcoords, state);

}

void drawStraightCylinder(const Point3f& a, const Point3f& b, float in_thickness, float out_thickness,
                          std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f>& texcoords, TurtleState &state) {

    connect(a, b, (b-a).normalized(), in_thickness, out_thickness, idealSmoothness(std::max(in_thickness, out_thickness)),
            positions, indices, texcoords);

    state.p = b;
    state.p_n = (b-a).normalized();
    state.in_thickness = out_thickness;
}

void connect(const Point3f &a, const Point3f &b, const Vector3f &a_n, float in_thickness, float out_thickness,
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

std::vector<Vector3f> circle(const Vector3f& p, const Vector3f& p_n, float radius, int smoothness) {

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

void push_circle(std::vector<Vector3f> &circle_positions, const Vector3f &p, std::vector<Vector3f> &positions,
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

NORI_NAMESPACE_END