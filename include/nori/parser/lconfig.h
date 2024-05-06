#pragma once

#include <core/common.h>
#include <parser/lsystem.h>
#include <parser/turtle.h>
#include <pcg32.h>
#include <stats/warp.h>
#include <bsdf/multidiffusemap.h>
#include <Eigen/Geometry>

NORI_NAMESPACE_BEGIN

class LGrammarConfig {

public:

    LGrammarConfig(float initial_width, float initial_length, float width_factor, float length_factor, float pitch_term, float yaw_term)
        : initial_width(initial_width), initial_length(initial_length), width_factor(width_factor),
          length_factor(length_factor), pitch_term(pitch_term), yaw_term(yaw_term) {}

    virtual void controlFrame(TurtleState& state, char c) { }
    virtual void controlYaw(TurtleState &state, char c) {  }
    virtual void controlPitch(float pitch, char c) {  }
    virtual void controlLength(TurtleState &state, char c) {  }
    virtual void controlThickness(TurtleState &state, char c) {  }
    virtual int pickRule(char c, float thickness, float length, int depth) { return 0; }
    virtual int colorIndex(char c) { return 0; }
    virtual int colorCount() { return 0; }
    virtual void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) { }

    float get_initial_width() const { return initial_width; }
    float get_initial_length() const { return initial_length; }
    float get_width_factor() const { return width_factor; }
    float get_length_factor() const { return length_factor; }
    float get_pitch_term() const { return pitch_term; }
    float get_yaw_term() const { return yaw_term; }

protected:
    float initial_width, initial_length, width_factor,
          length_factor, pitch_term, yaw_term;
};

class RealTreeConfig : public LGrammarConfig {

public:
    RealTreeConfig(pcg32& random, MultiDiffuseMap* map, float width_factor, float length_factor, float pitch_term, float yaw_term)
            : map(map),
              random(random),
              LGrammarConfig(1.7f, 8.0f, width_factor, length_factor, pitch_term, yaw_term){}

    int colorCount() override {
        return 2;
    }

    int colorIndex(char c) override {
        switch (c) {
            case 'F':
            case 'G':
                return 0;
            case 'H':
                return 1;
            default:
                throw NoriException("Unhandled color index");
        }
    }

    static int pick(float sample, float pa, float pb, float pc=0.f, float pd=0.f, float pe=0.f, float pf=0.f) {
        if(sample < pa) return 0;
        if(sample < pa + pb) return 1;
        if(sample < pa + pb + pc) return 2;
        if(sample < pa + pb + pc + pd) return 3;
        if(sample < pa + pb + pc + pd + pe) return 4;
        return 3;
    }

    Eigen::Matrix4f create_affine_matrix(float yaw, float pitch, const Vector3f& scale, const Vector3f& p) {

        Eigen::Affine3f transform;
        transform.setIdentity();
        transform = Eigen::DiagonalMatrix<float, 3>(scale) * transform;
        transform = Eigen::Translation<float, 3>(p.x(), p.y(), p.z()) * transform;
        /*transform = Eigen::AngleAxis<float>(angle, axis) * transform;
        t = Eigen::Translation<float, 3>(trans);*/
        return transform.matrix();
    }

    void controlFrame(TurtleState& state, char c) override {

        Vector3f dir = state.frame.n;

        // if true, it is okay to go down
        if(state.p.y() > 3 * initial_length || state.depth > 7) {
            return;
        }
        // otherwise, bump the direction up
        if(dir.y() < 0) {
            Vector3f bended = Vector3f(dir.x(), 1, dir.z()).normalized();
            state.frame = Frame(bended);
        }
    }

    void controlYaw(TurtleState &state, char c) override {
        state.yaw += Warp::lineToLogistic(random.nextFloat(), 0.06);
    }

    void controlPitch(TurtleState &state, char c) {
        state.pitch += Warp::lineToLogistic(random.nextFloat(), 0.06);
    }

    void controlLength(TurtleState &state, char c) override {
        state.length *= 1 + Warp::lineToLogistic(random.nextFloat(), 0.06);
    }

    void controlThickness(TurtleState &state, char c) override {
        if(state.depth <= 5) {
            state.out_thickness = (state.out_thickness + initial_width) / 2.0f;
        } else if(state.out_thickness < 0.1) {
            state.out_thickness = 0.1;
        } else {
            state.out_thickness *= (1 + Warp::lineToLogistic(random.nextFloat(), 0.06));
        }
    }

    int pickRule(char c, float thickness, float length, int depth) override {
        float sample = random.nextFloat();

        // Node =s FlowerBranch
        // Node =s FlowerBranch Depth -- Node; FlowerBranch Depth ++ Node; FlowerBranch Depth ++>>>>>> Node; FlowerBranch Depth ++<<<<<< Node
        // Node =s Branch Depth [wl++ Node][wl-- Node]; Branch Depth [wl++>>>>>> Node][wl-->>>>>> Node]
        // Node =s Branch Depth [wl++ Node][wl-->>>> Node][wl-- Node]
        // Node =s FlowerBranch Depth [wl++ Node][wl-->>>> Node][wl-- Node]
        if(c == 'N') {
            if(depth <= 3)  return pick(sample, 0.0, 0.0, 1.0, 0.0, 0.0);
            if(depth <= 5)  return pick(sample, 0.0, 0.6, 0.2, 0.2, 0.0);
            if(depth <= 8)  return pick(sample, 0.4, 0.0, 0.0, 0.0, 0.6);
            if(depth <= 12) return pick(sample, 0.8, 0.0, 0.0, 0.0, 0.2);
            return 1;
        }

        if(c == 'X') {
            return 1;
        }

        throw NoriException("Unhandled stochastic rule");
    }

    void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) override {

        controlFrame(state, c);

        if(state.random) {
            controlLength(state, c);
            controlPitch(state, c);
            controlYaw(state, c);
            controlThickness(state, c);
        }

        std::vector<Vector2f> temp;

        if(c == 'G' || c == 'F') {
            drawCylinder(state, positions, indices, temp);
        } else {
            Point3f p_advanced = state.p + state.p_n * 0.3f;
            auto trafo = Transform(
                    create_affine_matrix(state.yaw, state.pitch, Vector3f(0.5f), p_advanced));
            //drawMesh("assets/shape/flatcube.obj", trafo, positions, indices, temp);
        }

        int index = colorIndex(c);
        for(auto t : temp) {
            float x_mapped = map->map(t.x(), index);
            texcoords.push_back(Vector2f(x_mapped, t.y()));
        }
    }

protected:
    pcg32 random;
    MultiDiffuseMap* map = nullptr;
};

class Config6 : public LGrammarConfig {

public:
    Config6(pcg32& random, MultiDiffuseMap* map, float width_factor, float length_factor, float pitch_term, float yaw_term)
            : map(map),
              random(random),
              LGrammarConfig(0.3f, 2.0f, width_factor, length_factor, pitch_term, yaw_term){}

    int colorCount() override {
        return 2;
    }


    void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) override {

        std::vector<Vector2f> temp;
        drawCylinder(7, 0.01, random.nextFloat(),positions, indices, temp, state);

        int index = 0;
        for(auto t : temp) {
            float x_mapped = map->map(t.x(), index);
            texcoords.push_back(Vector2f(x_mapped, t.y()));
        }
    }

protected:
    pcg32 random;
    MultiDiffuseMap* map = nullptr;
};

NORI_NAMESPACE_END
