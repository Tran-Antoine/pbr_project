#pragma once

#include <core/common.h>
#include <core/vector.h>
#include <core/frame.h>
#include <core/transform.h>

NORI_NAMESPACE_BEGIN

struct TurtleState {

    Vector3f p;
    float yaw, pitch;
    Vector3f p_n;
    float in_thickness;
    float out_thickness;
    float length;
    bool random = false;
    int depth = 0;

};

int idealSmoothness(float radius);

void drawMesh(const std::string &filename, const Transform &trafo, std::vector<Vector3f> &positions,
              std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords);

void drawCylinder(float length, float yaw, float pitch, float in_thickness, float out_thickness,
                         std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f>& texcoords, TurtleState &state);

void drawCylinder(const Point3f& a, const Point3f& b, const Vector3f& a_n, const Vector3f& b_n, float in_thickness, float out_thickness,
                  std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f>& texcoords, TurtleState &state);

void drawCylinder(float length, float yaw, float pitch, float in_thickness, float out_thickness, int n_nodes, float variance, float sample,
                  std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f>& texcoords, TurtleState &state);

void drawStraightCylinder(const Point3f& a, const Point3f& b, float in_thickness, float out_thickness,
                          std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f>& texcoords, TurtleState &state);

void connect(const Point3f &a, const Point3f &b, const Vector3f &a_n, float in_thickness, float out_thickness,
                    int smoothness, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices,
                    std::vector<Vector2f> &texcoords);

std::vector<Vector3f> circle(const Vector3f& p, const Vector3f& p_n, float radius, int smoothness);

void push_circle(std::vector<Vector3f> &circle_positions, const Vector3f &p, std::vector<Vector3f> &positions,
                        std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords,
                        bool fill_circle);

NORI_NAMESPACE_END