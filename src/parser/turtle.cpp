#pragma once

#include <parser/turtle.h>
#include <parser/obj.h>
#include <stats/warp.h>

NORI_NAMESPACE_BEGIN

int idealSmoothness(float radius) {
    return (int) (radius * 100);
}

void drawMesh(const std::string &filename, const Transform &trafo,
              PosVec ps, IndVec is, std::vector<Vector2f> &ts) {


    int starting_index = ps.size();

    auto props = PropertyList();
    props.setString("filename", filename);
    props.setTransform("toWorld", trafo);
    props.setBoolean("no-print", true);

    auto mesh = WavefrontOBJ(props);

    auto ps_ = mesh.getVertexPositions();
    for(int i = 0; i < ps_.cols(); i++) {
        ps.push_back(ps_.col(i));
    }

    auto is_ = mesh.getIndices();
    for(int i = 0; i < is_.cols(); i++) {
        auto column = is_.col(i);
        is.push_back(starting_index + column.x());
        is.push_back(starting_index + column.y());
        is.push_back(starting_index + column.z());
    }

    auto ts_ = mesh.getVertexTexCoords();
    for(int i = 0; i < ts_.cols(); i++) {
        ts.push_back(ts_.col(i));
    }
}

void drawCylinder(TurtleState &state, PosVec ps, IndVec is, TexVec ts) {

    Vector3f a = state.p;
    Vector3f a_n = state.p_n;
    Vector3f b = state.forward();
    Vector3f b_n = state.normalOut();

    float in_thick = state.in_thickness, out_thick = state.out_thickness;

    drawCylinder(a, b, a_n, b_n, in_thick, out_thick, ps, is, ts, state);
}

void drawCylinder(const Point3f& a, const Point3f& b, const Vector3f& a_n, const Vector3f& b_n, float in_thick, float out_thick,
                  PosVec ps, IndVec is, TexVec ts, TurtleState &state) {

    if(abs(a_n.dot(b_n)) < 0.4) {
        // start with the out thick right away
        drawCylinder(a, b, b_n, b_n, out_thick, out_thick, ps, is, ts, state);
    }

    connect(a, b, a_n, in_thick, out_thick, idealSmoothness(std::max(in_thick, out_thick)),
            ps, is, ts);
    state.p = b;
    state.p_n = b_n;
    state.in_thickness = out_thick;
    state.frame = Frame(b_n);
    state.resetOrientation();
}

static void addNoise(float& yaw, float& pitch, float sample, float variance) {
    yaw *= 1 + Warp::lineToLogistic(sample, variance);
    pitch *= 1 + Warp::lineToLogistic(sample, variance);
}

void drawCylinder(int n_nodes, float variance, float sample,
                  PosVec ps, IndVec is, TexVec ts, TurtleState &state) {

    float original_length = state.length;
    float in_thick = state.in_thickness, out_thick = state.out_thickness;
    float yaw = state.yaw, pitch = state.pitch;
    float dl = original_length / (float) n_nodes;
    float dt = (out_thick - in_thick) / (float) n_nodes;

    Vector3f end_point = state.forward();
    Vector3f end_out_normal = state.normalOut();
    float    end_thickness = state.out_thickness;

    state.out_thickness = state.in_thickness;
    state.length = dl;

    for(int i = 0; i < n_nodes - 1; i++) {
        addNoise(yaw, pitch, sample, variance);

        state.yaw = yaw;
        state.pitch = pitch;
        state.out_thickness += dt;

        drawCylinder(state, ps, is, ts);
        yaw = 0; pitch = M_PI / 2;
    }

    drawCylinder(state.p, end_point, state.p_n, end_out_normal, state.in_thickness, end_thickness, ps, is, ts, state);
    state.length = original_length;
}

void drawStraightCylinder(const Point3f& a, const Point3f& b, float in_thick, float out_thick,
                          PosVec ps, IndVec is, TexVec ts, TurtleState &state) {

    Vector3f normal = (b-a).normalized();
    drawCylinder(a, b, normal, normal, in_thick, out_thick, ps, is, ts, state);
}

void connect(const Point3f &a, const Point3f &b, const Vector3f &a_n, float in_thick, float out_thick,
                    int smoothness, PosVec ps, IndVec is,
                    std::vector<Vector2f> &ts) {

    Vector3f b_n = (b-a).normalized(); // normal of b must be according to the cylinder's direction

    std::vector<Vector3f> from_circle = circle(a, a_n, in_thick, smoothness);
    std::vector<Vector3f> to_circle   = circle(b, b_n, out_thick, smoothness);

    int index_pointer = ps.size(); // save pointer before adding new vertices
    push_circle(from_circle, a, ps, is, ts, false);
    push_circle(to_circle, b, ps, is, ts, true);

    int n_points = from_circle.size() + 1; // +1 due to the center point

    // goal: form two triangles to shape the rectangle of points (a0, a1, b0, b1)
    int i0 = index_pointer + 1;
    int j0 = i0 + n_points;

    // For now we only allow smoothness to control the number of points, not the "length" resolution
    for(int i = 1; i < n_points - 1; i++) {

        int i1 = i0 + 1;
        int j1 = j0 + 1;

        is.push_back(i0);
        is.push_back(i1);
        is.push_back(j1);

        is.push_back(i0);
        is.push_back(j1);
        is.push_back(j0);

        i0++;
        j0++;
    }

    is.push_back(i0);
    is.push_back(index_pointer + 1);
    is.push_back(index_pointer + 1 + n_points);

    is.push_back(i0);
    is.push_back(index_pointer + 1 + n_points);
    is.push_back(j0);
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

void push_circle(PosVec circle_ps, const Vector3f &p, PosVec ps,
                        IndVec is, std::vector<Vector2f> &ts,
                        bool fill_circle) {

    int center_pointer = ps.size();
    int head = ps.size() + 1;
    int edge_pointer = head;

    float y_texture = fill_circle ? 1.f : 0.f;

    ps.push_back(p);
    ts.push_back(Vector2f(0.5f, y_texture)); // the value of this one doesn't really matter

    Vector3f edge0 = circle_ps[0];
    ps.push_back(edge0);
    ts.push_back(Vector2f(0.0f, y_texture));

    for(int i = 1; i <= circle_ps.size() - 1; i++) {

        float x_texture = (float) i / (float) circle_ps.size();

        Vector3f edge1 = circle_ps[i];
        ps.push_back(edge1);
        ts.push_back(Vector2f(x_texture, y_texture));


        if(fill_circle) {
            is.push_back(center_pointer);
            is.push_back(edge_pointer);
            is.push_back(edge_pointer+1);
        }

        edge_pointer++;
    }

    if(fill_circle) {
        is.push_back(center_pointer);
        is.push_back(edge_pointer);
        is.push_back(head);
    }
}

NORI_NAMESPACE_END