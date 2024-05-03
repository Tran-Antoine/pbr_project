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

    virtual float randomizeYaw(float yaw, char c) { return yaw; }
    virtual float randomizePitch(float pitch, char c) { return pitch; }
    virtual float randomizeLength(float length, char c) { return length; }
    virtual float randomizeThickness(float thickness, char c) { return thickness; }
    virtual int pickRule(char c, float thickness, float length, int depth) { return 0; }
    virtual int colorIndex(char c) { return 0; }
    virtual int colorCount() { return 0; }
    virtual void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) {}

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

class Config0 : public LGrammarConfig {

public:
    Config0(pcg32& random, float width_factor, float length_factor, float pitch_term, float yaw_term) : random(random),
            LGrammarConfig(0.5f, 1.0f, width_factor, length_factor, pitch_term, yaw_term){}

    float randomizeYaw(float yaw, char c) override {
        return yaw + M_PI / 5 * Warp::lineToLogistic(random.nextFloat(), 0.6);
    }

    float randomizePitch(float pitch, char c) override {

        if(pitch < 0.f) {
            return pitch / 2.f + 0.5f;
        }

        return pitch * (1 + 0.2f * Warp::lineToLogistic(random.nextFloat(), 0.2));
    }

    float randomizeLength(float length, char c) override {

        return length * (1 + 0.3f * Warp::lineToLogistic(random.nextFloat(), 0.6));
    }
    float randomizeThickness(float thickness, char c) override {

        float randomized = thickness * (1 + 0.1f * Warp::lineToLogistic(random.nextFloat(), 0.6));

        if(thickness < 0.22) {
            randomized /= 1.5f;
        }
        return randomized;
    }

    static int pick(float sample, float pa, float pb, float pc=0.f, float pd=0.f, float pe=0.f, float pf=0.f) {
        if(sample < pa) return 0;
        if(sample < pa + pb) return 1;
        if(sample < pa + pb + pc) return 2;
        if(sample < pa + pb + pc + pd) return 3;
        if(sample < pa + pb + pc + pd + pe) return 4;
        return 3;
    }

    int pickRule(char c, float thickness, float length, int depth) override {
        float sample = random.nextFloat();
        if(depth <= 0) {
            return pick(sample, 0.0f, 0.35f, 0.35f, 0.3f);
        } else if(depth <= 2) {
            return pick(sample, 0.0f, 0.0f, 0.0f, 1.f);
        } else if(depth <= 4) {
            return pick(sample, 0.0f, 0.2f, 0.2f, 0.6f);
        } else if(depth <= 6) {
            return pick(sample, 0.0f, 0.1f, 0.1f, 0.8f);
        } else if(depth <= 8) {
            return pick(sample, 0.2f, 0.2f, 0.2f, 0.4f);
        } else {
            return pick(sample, 0.5f, 0.0f, 0.0f, 0.5f);
        }
    }

    int colorIndex(char c) override {
        switch (c) {
            case 'F': return 1;
            case 'G': return 0;
            default: throw NoriException("Unsupported drawing character");
        }
    }

    int colorCount() override {
        return 2;
    }

    void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) override {
        bool randomization = state.random;

        float length = state.length;
        float yaw = state.yaw, pitch = state.pitch;
        float in_thickness = state.in_thickness;
        float out_thickness = state.out_thickness;

        if(randomization) {
            length = randomizeLength(length, c);
            yaw = randomizeYaw(yaw, c);
            pitch = randomizePitch(pitch, c);
            out_thickness = randomizeThickness(out_thickness, c);
        }

        drawCylinder(length, yaw, pitch, in_thickness, out_thickness, positions, indices, texcoords, state);
    }

protected:
    pcg32 random;
};

class Config1 : public LGrammarConfig {

    public:
        Config1(pcg32& random, float width_factor, float length_factor, float pitch_term, float yaw_term)
            : random(random),
              LGrammarConfig(0.5f, 1.0f, width_factor, length_factor, pitch_term, yaw_term){}

        float randomizeYaw(float yaw, char c) override {
            return yaw + M_PI / 5 * Warp::lineToLogistic(random.nextFloat(), 0.6);
        }

        float randomizePitch(float pitch, char c) override {

            if(pitch < 0.f) {
                return pitch / 2.f + 0.5f;
            }

            return pitch * (1 + 0.2f * Warp::lineToLogistic(random.nextFloat(), 0.2));
        }

        float randomizeLength(float length, char c) override {

            return length * (1 + 0.3f * Warp::lineToLogistic(random.nextFloat(), 0.6));
        }
        float randomizeThickness(float thickness, char c) override {

            float randomized = thickness * (1 + 0.1f * Warp::lineToLogistic(random.nextFloat(), 0.6));

            if(thickness < 0.22) {
                randomized /= 1.5f;
            }
            return randomized;
        }

        static int pick(float sample, float pa, float pb, float pc=0.f, float pd=0.f, float pe=0.f, float pf=0.f) {
            if(sample < pa) return 0;
            if(sample < pa + pb) return 1;
            if(sample < pa + pb + pc) return 2;
            if(sample < pa + pb + pc + pd) return 3;
            if(sample < pa + pb + pc + pd + pe) return 4;
            return 3;
        }

        int pickRule(char c, float thickness, float length, int depth) override {

            // TODO: move "drawing" algorithm to the configuration so it can draw differently depending on the character

            float sample = random.nextFloat();
            if(depth <= 0) {
                return pick(sample, 0.0f, 0.35f, 0.35f, 0.3f);
            } else if(depth <= 2) {
                return pick(sample, 0.0f, 0.0f, 0.0f, 1.f);
            } else if(depth <= 4) {
                return pick(sample, 0.0f, 0.2f, 0.2f, 0.6f);
            } else if(depth <= 6) {
                return pick(sample, 0.0f, 0.1f, 0.1f, 0.8f);
            } else if(depth <= 8) {
                return pick(sample, 0.2f, 0.2f, 0.2f, 0.4f);
            } else {
                return pick(sample, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f);
            }
        }

        int colorIndex(char c) override {
            switch (c) {
                case 'F': return 1;
                case 'G': return 0;
                default: throw NoriException("Unsupported drawing character");
            }
        }

        int colorCount() override {
            return 2;
        }

        void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) override {
            bool randomization = state.random;

            float length = state.length;
            float yaw = state.yaw, pitch = state.pitch;
            float in_thickness = state.in_thickness;
            float out_thickness = state.out_thickness;

            if(randomization) {
                length = randomizeLength(length, c);
                yaw = randomizeYaw(yaw, c);
                pitch = randomizePitch(pitch, c);
                out_thickness = randomizeThickness(out_thickness, c);
            }

            drawCylinder(length, yaw, pitch, in_thickness, out_thickness, positions, indices, texcoords, state);
        }

    protected:
        pcg32 random;
    };

class Config2 : public LGrammarConfig {

public:
    Config2(MultiDiffuseMap* map, float width_factor, float length_factor, float pitch_term, float yaw_term)
        : map(map),
          LGrammarConfig(0.5f, 1.0f, width_factor, length_factor, pitch_term, yaw_term){}

    int colorCount() override {
        return 2;
    }


    Eigen::Matrix4f create_affine_matrix(float yaw, float pitch, const Vector3f& scale, const Vector3f& p) {

        /*Vector3f direction = directional(pitch, yaw);
        Vector3f test = Vector3f(-direction.y(), direction.x(), 0).normalized();
        Vector3f test2 = direction.cross(test).normalized();
        Frame frame(test, test2, direction);*/


        Eigen::Affine3f transform;
        transform.setIdentity();
        transform = Eigen::DiagonalMatrix<float, 3>(scale) * transform;
        transform = Eigen::Translation<float, 3>(p.x(), p.y(), p.z()) * transform;
        /*transform = Eigen::AngleAxis<float>(angle, axis) * transform;
        t = Eigen::Translation<float, 3>(trans);*/
        return transform.matrix();
    }

    void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) override {

        float length = state.length;
        float yaw = state.yaw, pitch = state.pitch;
        float in_thickness = state.in_thickness;
        float out_thickness = state.out_thickness;

        std::vector<Vector2f> temp;

        if (c == 'F') {
            drawCylinder(length, yaw, pitch, in_thickness, out_thickness, positions, indices, temp, state);
        } else {
            auto trafo = Transform(create_affine_matrix(state.yaw, state.pitch, Vector3f(state.out_thickness, state.length, state.out_thickness), state.p));
            drawMesh("assets/shape/sphere.obj", trafo, positions, indices, temp);
        }

        int index = c == 'F' ? 0 : 1;
        for(auto t : temp) {
            float x_mapped = map->map(t.x(), index);
            texcoords.push_back(Vector2f(x_mapped, t.y()));
        }
    }

protected:
    MultiDiffuseMap* map = nullptr;
};

class Config3 : public LGrammarConfig {

public:
    Config3(MultiDiffuseMap* map, float width_factor, float length_factor, float pitch_term, float yaw_term)
            : map(map),
              LGrammarConfig(0.5f, 1.0f, width_factor, length_factor, pitch_term, yaw_term){}

    int colorCount() override {
        return 2;
    }


    Eigen::Matrix4f create_affine_matrix(float yaw, float pitch, const Vector3f& scale, const Vector3f& p) {

        /*Vector3f direction = directional(pitch, yaw);
        Vector3f test = Vector3f(-direction.y(), direction.x(), 0).normalized();
        Vector3f test2 = direction.cross(test).normalized();
        Frame frame(test, test2, direction);*/


        Eigen::Affine3f transform;
        transform.setIdentity();
        transform = Eigen::DiagonalMatrix<float, 3>(scale) * transform;
        transform = Eigen::Translation<float, 3>(p.x(), p.y(), p.z()) * transform;
        /*transform = Eigen::AngleAxis<float>(angle, axis) * transform;
        t = Eigen::Translation<float, 3>(trans);*/
        return transform.matrix();
    }

    void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) override {

        float length = state.length;
        float yaw = state.yaw, pitch = state.pitch;
        float in_thickness = state.in_thickness;
        float out_thickness = state.out_thickness;

        std::vector<Vector2f> temp;

        if (c == 'F') {
            drawCylinder(length, yaw, pitch, in_thickness, out_thickness, positions, indices, temp, state);
        } else {
            Point3f p_advanced = state.p + state.p_n * 1.7f*state.out_thickness;
            auto trafo = Transform(create_affine_matrix(state.yaw, state.pitch, Vector3f(10.f*state.out_thickness), p_advanced));
            drawMesh("assets/shape/sphere.obj", trafo, positions, indices, temp);
        }

        int index = c == 'F' ? 0 : 1;
        for(auto t : temp) {
            float x_mapped = map->map(t.x(), index);
            texcoords.push_back(Vector2f(x_mapped, t.y()));
        }
    }

protected:
    MultiDiffuseMap* map = nullptr;
};

class Config4 : public LGrammarConfig {

public:
    Config4(MultiDiffuseMap* map, float width_factor, float length_factor, float pitch_term, float yaw_term)
            : map(map),
              LGrammarConfig(0.5f, 1.0f, width_factor, length_factor, pitch_term, yaw_term){}

    int colorCount() override {
        return 2;
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

    void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) override {

        float length = state.length;
        float yaw = state.yaw, pitch = state.pitch;
        float out_thickness = state.out_thickness;

        std::vector<Vector2f> temp;

        if (c == 'F' || c == 'H') {

            Point3f a = state.p;
            Point3f b = a + directional(pitch, yaw) * length;
            drawStraightCylinder(a, b, out_thickness, out_thickness, positions, indices, temp, state);
        } else {
            Point3f p_advanced = state.p + state.p_n * 1.5f*state.out_thickness;
            auto trafo = Transform(create_affine_matrix(state.yaw, state.pitch, Vector3f(2.f*state.out_thickness), p_advanced));
            drawMesh("assets/shape/sphere.obj", trafo, positions, indices, temp);
        }

        int index = c == 'F' || c == 'H' ? 0 : 1;
        for(auto t : temp) {
            float x_mapped = map->map(t.x(), index);
            texcoords.push_back(Vector2f(x_mapped, t.y()));
        }
    }

protected:
    MultiDiffuseMap* map = nullptr;
};

class Config5 : public LGrammarConfig {

public:
    Config5(pcg32& random, MultiDiffuseMap* map, float width_factor, float length_factor, float pitch_term, float yaw_term)
            : map(map),
              random(random),
              LGrammarConfig(0.5f, 1.0f, width_factor, length_factor, pitch_term, yaw_term){}

    int colorCount() override {
        return 2;
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

    float randomizeYaw(float yaw, char c) override {
        return yaw + M_PI / 5 * Warp::lineToLogistic(random.nextFloat(), 0.6);
    }

    float randomizePitch(float pitch, char c) override {

        if(pitch < 0.f) {
            return pitch / 2.f + 0.5f;
        }

        return pitch * (1 + 0.2f * Warp::lineToLogistic(random.nextFloat(), 0.2));
    }

    float randomizeLength(float length, char c) override {

        return length * (1 + 0.3f * Warp::lineToLogistic(random.nextFloat(), 0.6));
    }
    float randomizeThickness(float thickness, char c) override {

        float randomized = thickness * (1 + 0.1f * Warp::lineToLogistic(random.nextFloat(), 0.6));

        if(thickness < 0.22) {
            randomized /= 1.5f;
        }
        return randomized;
    }

    int pickRule(char c, float thickness, float length, int depth) override {
        float sample = random.nextFloat();

        if(c == 'B') {
            if(depth <= 2) return pick(sample, 0.0, 0.5, 0.5);
            if(depth <= 5) return pick(sample, 0.2, 0.6, 0.2);
            else           return pick(sample, 0.7, 0.3, 0.0);
        }

        if(c == 'F') {
            if(depth <= 1) return pick(sample, 0.0, 0.2, 0.8);
            if(depth <= 5) return pick(sample, 0.1, 0.1, 0.4, 0.4);
            if(depth <= 8) return pick(sample, 0.2, 0.1, 0.4, 0.3);
            else           return pick(sample, 0.5, 0.0, 0.4, 0.1);
        }

        throw NoriException("Unhandled stochastic rule");
    }

    void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) override {

        float length = state.length;
        float yaw = state.yaw, pitch = state.pitch;
        float in_thickness = state.in_thickness;
        float out_thickness = state.out_thickness;

        if(state.random) {
            length = randomizeLength(length, c);
            yaw = randomizeYaw(yaw, c);
            pitch = randomizePitch(pitch, c);
            out_thickness = randomizeThickness(out_thickness, c);
        }

        std::vector<Vector2f> temp;
        drawCylinder(length, yaw, pitch, in_thickness, out_thickness, positions, indices, temp, state);

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
