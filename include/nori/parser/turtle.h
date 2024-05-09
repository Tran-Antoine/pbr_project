#pragma once

#include <core/common.h>
#include <core/vector.h>
#include <core/frame.h>
#include <core/transform.h>

NORI_NAMESPACE_BEGIN

using PosVec = std::vector<Vector3f>&;
using IndVec = std::vector<uint32_t>&;
using TexVec = std::vector<Vector2f>&;


struct TurtleState {

    Vector3f p;
    Frame frame;
    float yaw, pitch;
    Vector3f p_n;
    float in_thickness;
    float out_thickness;
    float length;
    bool random = false;
    int depth = 0;

    void resetOrientation() {
        yaw = 0;
        pitch = M_PI / 2;
    }

    Point3f forward() {
        Vector3f localDirection = directional(pitch, yaw);
        std::swap(localDirection.y(), localDirection.z());

        Vector3f toWorld = frame.toWorld(localDirection);

        return p + length * toWorld;
    }

    Point3f forward(float len) {
        Vector3f localDirection = directional(pitch, yaw);
        std::swap(localDirection.y(), localDirection.z());
        Vector3f toWorld = frame.toWorld(localDirection);
        return p + len * toWorld;
    }

    Vector3f normalOut() {
        return (forward() - p).normalized();
    }
};

int idealSmoothness(float radius);

void drawMesh(const std::string &filename, const Transform &trafo,
              PosVec ps, IndVec is, TexVec ts);

void drawPopulatedCylinder(TurtleState &state, PosVec ps, IndVec is, TexVec ts);

void drawCylinder(TurtleState &state, PosVec ps, IndVec is, TexVec ts);

void drawCylinder(const Point3f& a, const Point3f& b, const Vector3f& a_n, const Vector3f& b_n, float in_thickness, float out_thickness,
                  PosVec ps, IndVec is, TexVec ts, TurtleState &state);

void drawCylinder(int n_nodes, float variance, float sample,
                  PosVec ps, IndVec is, TexVec ts, TurtleState &state);

void drawStraightCylinder(const Point3f& a, const Point3f& b, float in_thickness, float out_thickness,
                          PosVec ps, IndVec is, TexVec ts, TurtleState &state);

void connect(const Point3f &a, const Point3f &b, const Vector3f &a_n, float in_thickness, float out_thickness, int smoothness,
             PosVec ps, IndVec is, TexVec ts);

std::vector<Vector3f> circle(const Vector3f& p, const Vector3f& p_n, float radius, int smoothness);

void push_circle(PosVec circle_ps, const Vector3f &p, PosVec ps,
                 IndVec is, TexVec ts, bool fill_circle);

NORI_NAMESPACE_END