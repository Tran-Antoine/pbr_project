#pragma once

#include <core/common.h>
#include <parser/lsystem.h>
#include <parser/turtle.h>
#include <pcg32.h>
#include <stats/warp.h>
#include <bsdf/multidiffusemap.h>
#include <Eigen/Geometry>
#include <core/bbox.h>
#include <stats/stripeperlin.h>

NORI_NAMESPACE_BEGIN

class LGrammarConfig {

public:

    LGrammarConfig(float initial_width, float initial_length, float width_factor, float length_factor, float pitch_term, float yaw_term)
        : initial_width(initial_width), initial_length(initial_length), width_factor(width_factor),
          length_factor(length_factor), pitch_term(pitch_term), yaw_term(yaw_term) {}

    virtual void controlFrame(TurtleState& state, char c) { }
    virtual void controlYaw(TurtleState &state, char c) {  }
    virtual void controlPitch(TurtleState &state, char c) {  }
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
    RealTreeConfig(pcg32& random, MultiDiffuseMap* map, float width_factor, float length_factor, float pitch_term, float yaw_term,
                   const Transform& trafo)
            : map(map),
              random(random),
              trafo(trafo),
              LGrammarConfig(2.0f, 5.0f, width_factor, length_factor, pitch_term, yaw_term){}

    int colorCount() override {
        return 4;
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
        /*if(dir.y() < -1) {
            Vector3f bended = Vector3f(dir.x()/2.f, 1, dir.z()/2.f).normalized();
            Vector3f s = Vector3f(-bended.y(), bended.x(), 0).normalized();
            Vector3f t = bended.cross(s).normalized();
            state.frame = Frame(s,t,bended);
        }*/
    }

    void controlYaw(TurtleState &state, char c) override {
        if(state.depth < 4) return;
        state.yaw += Warp::lineToLogistic(random.nextFloat(), 0.06);
    }

    void controlPitch(TurtleState &state, char c) {
        state.pitch += Warp::lineToLogistic(random.nextFloat(), 0.06);
    }

    void controlLength(TurtleState &state, char c) override {
        if(state.depth < 4) return;
        if(state.depth > 7) {
            state.length *= 0.9;
        }
        state.length *= 1 + Warp::lineToLogistic(random.nextFloat(), 0.06);
    }

    void controlThickness(TurtleState &state, char c) override {
        if(state.depth < 4) return;
        if(state.out_thickness < 0.2) {
            state.out_thickness = 0.2;
        } else {
            state.out_thickness *= (1 + Warp::lineToLogistic(random.nextFloat(), 0.01));
        }
    }

    int pickRule(char c, float thickness, float length, int depth) override {
        float sample = random.nextFloat();

        //Node =s FlowerBranch
        //Node =s Change direction
        //Node =s Split in two
        //Node =s Split in two, terminates one branch
        //Node =s Split in three

        if(c == 'K') {
            if(depth <= 4)  return pick(sample, 0.0, 0.0, 0.8, 0.0, 0.2, 0.0);
            if(depth <= 6)  return pick(sample, 0.0, 0.4, 0.3, 0.2, 0.0, 0.0);
            if(depth <= 8)  return pick(sample, 0.0, 0.1, 0.4, 0.4, 0.1, 0.0);
            return 0;
            if(depth <= 8)  return pick(sample, 0.1, 0.0, 0.0, 0.3, 0.2, 0.5);
            if(depth <= 12) return pick(sample, 0.8, 0.0, 0.0, 0.2, 0.0, 0.0);
            return 1;
        }

        throw NoriException("Unhandled stochastic rule");
    }

    void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) override {

        controlFrame(state, c);

        if(state.depth > 7) {
            state.length *= 0.9;
        }

        if(state.random) {
            controlLength(state, c);
            controlPitch(state, c);
            controlYaw(state, c);
            controlThickness(state, c);
        }

        std::vector<Vector2f> temp;

        if (state.depth >= 3) {
            for(int i = 0; i < 15; i++) {
                float t = random.nextFloat() * state.length;
                float dx = 2 * random.nextFloat() - 1;
                float dy = 2 * random.nextFloat() - 1;
                float dz = 2 * random.nextFloat() - 2.5;

                Point3f pos = state.forward(t) + 1.0f * Vector3f(dx, dy, dz);

                drawMesh("assets/shape/sphere_low.obj", create_affine_matrix(0, 0, 0.22, pos), positions, indices, temp);
                flower_anchors.push_back(pos);
                flower_bounds.expandBy(1.1 * pos);
            }
            for(auto t : temp) {
                float x_mapped = map->map(t.x(), 1 + (int) (2 * random.nextFloat()));
                texcoords.push_back(Vector2f(x_mapped, t.y()));
            }
            temp.clear();
        }

        if(c == 'G' || c == 'F') {
            drawCylinder(state,positions, indices, temp);
        }

        int index = 0;
        for(auto t : temp) {
            float x_mapped = map->map(t.x(), index);
            texcoords.push_back(Vector2f(x_mapped, t.y()));
        }
    }

    std::vector<Point3f> flower_anchors;
    std::vector<Point3f> bg_anchors;
    BoundingBox3f flower_bounds;
    int counter = 0;

protected:
    Transform trafo;
    pcg32 random;
    MultiDiffuseMap* map = nullptr;

};

class OneBranchConfig : public LGrammarConfig {

public:
    OneBranchConfig(pcg32& random, MultiDiffuseMap* map, float width_factor, float length_factor, float pitch_term, float yaw_term,
                    const Transform& trafo)
                : map(map),
                  random(random),
                  trafo(trafo),
                  LGrammarConfig(0.05f, 1.0f, width_factor, length_factor, pitch_term, yaw_term){}

        int colorCount() override {
            return 2;
        }

        int colorIndex(char c) override {
            switch (c) {
                case 'F':
                    return 0;
                case 'H':
                    return 1;
                default:
                    throw NoriException("Unhandled color index");
            }
        }

        Eigen::Matrix4f create_affine_matrix(float yaw, float pitch, const Vector3f& scale, const Vector3f& p) {

            Eigen::Affine3f transform;
            transform.setIdentity();
            transform = Eigen::AngleAxis<float>(0.4, Vector3f(0, 0, 1)) * transform;
            transform = Eigen::DiagonalMatrix<float, 3>(scale) * transform;
            transform = Eigen::Translation<float, 3>(p.x(), p.y(), p.z()) * transform;

            return transform.matrix();
        }


        void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) override {

            std::vector<Vector2f> temp;

            if(state.depth >= 4) {
                int n_divisions = 10;
                for(int i = 0; i < n_divisions; i++) {
                    float t = state.length * i / (n_divisions + 1);

                    Point3f current_point = state.forward(t);
                    float dx = random.nextFloat() * 2 - 1;
                    float dy = random.nextFloat() * 2 - 1;
                    float dz = random.nextFloat() * 2 - 1;

                    Point3f flower_point = current_point + Vector3f(0.2, 0.0, 0.0);
                    float scale = 20.f;
                    drawMesh("assets/shape/flower/petal_lowres.obj",
                             create_affine_matrix(0, 0, scale, flower_point),
                             positions, indices, temp);

                }
                int index = 1;
                for(auto t : temp) {
                    float x_mapped = map->map(t.x(), index);
                    texcoords.push_back(Vector2f(x_mapped, t.y()));
                }
                temp.clear();
            }

            if(c == 'G' || c == 'F') {
                drawCylinder(state,positions, indices, temp);
            }
            int index = 0;
            for(auto t : temp) {
                float x_mapped = map->map(t.x(), index);
                texcoords.push_back(Vector2f(x_mapped, t.y()));
            }
        }

    protected:
        Transform trafo;
        pcg32 random;
        MultiDiffuseMap* map = nullptr;

    };

class Config6 : public LGrammarConfig {

public:
    Config6(pcg32& random, MultiDiffuseMap* map, float width_factor, float length_factor, float pitch_term, float yaw_term)
            : map(map),
              random(random),
              LGrammarConfig(0.3f, 2.5f, width_factor, length_factor, pitch_term, yaw_term){}

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

class OneTrunkConfig : public LGrammarConfig {

public:
    OneTrunkConfig(pcg32& random, MultiDiffuseMap* map, float width_factor, float length_factor, float pitch_term, float yaw_term,
                        const Transform& trafo)
                : map(map),
                  random(random),
                  trafo(trafo),
                  LGrammarConfig(0.8f, 2.0f, width_factor, length_factor, pitch_term, yaw_term){}

    int colorCount() override {
        return 1;
    }

    static PerlinGenerator* noiseGen(int res_x, int res_z, uint64_t initstate, uint64_t initseq) {
        return new PerlinGenerator(res_x, res_z, initstate, initseq);
    }

    void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) override {

        float max_depth = 5;

        if(state.depth >= max_depth) {
            drawCylinder(state, positions, indices, texcoords);
            return;
        }

        int BASE_RES_X  = 8;
        int BASE_RES_Z  = 8;
        uint64_t SEED_STATE = 3227124;
        uint64_t SEED_SEQ = 10951913;
        FBM fbm(BASE_RES_X, BASE_RES_Z, 5, 0.35, 2.0, SEED_STATE, SEED_SEQ);
        fbm.init_generators();


        drawCylinder(state, fbm, state.depth / max_depth, state.depth / max_depth + 1 / max_depth,Vector2i(128, 128), positions, indices, texcoords);
    }

protected:
    Transform trafo;
    pcg32 random;
    MultiDiffuseMap* map = nullptr;

};

class TreeIsland : public LGrammarConfig {

public:
    TreeIsland(pcg32& random, MultiDiffuseMap* map, float width_factor, float length_factor, float pitch_term, float yaw_term,
                   const Transform& trafo)
            : map(map),
              random(random),
              trafo(trafo),
              LGrammarConfig(2.5f, 6.0f, width_factor, length_factor, pitch_term, yaw_term){}

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

        // do nothing
    }

    void controlYaw(TurtleState &state, char c) override {
        if(state.depth < 4) return;
        state.yaw += Warp::lineToLogistic(random.nextFloat(), 0.06);
    }

    void controlPitch(TurtleState &state, char c) override {
        if(state.depth < 3) return;
        state.pitch += Warp::lineToLogistic(random.nextFloat(), 0.06);
    }

    void controlLength(TurtleState &state, char c) override {
        if(state.depth < 4) return;
        if(state.depth > 7) {
            state.length *= 0.9;
        }
        state.length *= 1 + Warp::lineToLogistic(random.nextFloat(), 0.06);
    }

    void controlThickness(TurtleState &state, char c) override {
        if(state.depth < 3) return;
        state.out_thickness *= 0.85;
        state.out_thickness *= (1 + Warp::lineToLogistic(random.nextFloat(), 0.01));
        if(state.depth > 4) {
            state.out_thickness = 0.0001;
        }
    }

    int pickRule(char c, float thickness, float length, int depth) override {
        float sample = random.nextFloat();

        //Node =s FlowerBranch
        //Node =s Change direction
        //Node =s Split in two
        //Node =s Split in two, terminates one branch
        //Node =s Split in three

        if(c == 'K') {
            if(depth <= 4)  return pick(sample, 0.0, 0.0, 0.8, 0.0, 0.2, 0.0);
            if(depth <= 6)  return pick(sample, 0.0, 0.4, 0.3, 0.2, 0.0, 0.0);
            if(depth <= 8)  return pick(sample, 0.0, 0.1, 0.4, 0.4, 0.1, 0.0);
            //if(depth <= 8)  return pick(sample, 0.1, 0.0, 0.0, 0.3, 0.2, 0.5);
            if(depth <= 12) return pick(sample, 0.8, 0.0, 0.0, 0.2, 0.0, 0.0);
            return 0;
        }

        throw NoriException("Unhandled stochastic rule");
    }

    void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) override {

        controlFrame(state, c);

        // try reducing width much more after few depth

        if(state.random) {
            controlLength(state, c);
            controlPitch(state, c);
            controlYaw(state, c);
            controlThickness(state, c);
        }

        std::vector<Vector2f> temp;

        if (state.depth >= 3) {
            for(int i = 0; i < 15; i++) {

                float t = random.nextFloat() * state.length;
                float dx = 2 * random.nextFloat() - 1;
                float dy = 2 * random.nextFloat() - 1;
                float dz = 2 * random.nextFloat() - 2;

                Point3f cloud_pos = state.forward(t) + 1.3f * Vector3f(dx, dy, dz);


                if(i < 10) {
                    Point3f pos = state.forward(t) + 0.6f * Vector3f(dx, dy, dz);
                    drawMesh("assets/shape/sphere_low.obj", create_affine_matrix(0, 0,  0.01, pos), positions, indices, temp);
                }

                flower_anchors.push_back(cloud_pos);
                flower_bounds.expandBy(1.1 * cloud_pos);
            }
            for(auto t : temp) {
                float x_mapped = map->map(t.x(), 1 + (int) (1 * random.nextFloat()));
                texcoords.push_back(Vector2f(x_mapped, t.y()));
            }
            temp.clear();
        }

        if(c == 'G' || c == 'F' || c == 'H') {
            float max_depth = 3;

            if(state.depth >= max_depth) {
                if(state.depth <= 11) {
                    if (state.depth <= 7 || state.forward().y() > 19 ||
                        state.forward().x() < 0) {
                        drawCylinder(state, positions, indices, temp);
                    }
                }
            } else {
                int BASE_RES_X  = 6;
                int BASE_RES_Z  = 6;
                uint64_t SEED_STATE = 3227124;
                uint64_t SEED_SEQ = 10951913;
                FBM fbm(BASE_RES_X, BASE_RES_Z, 5, 0.35, 2.0, SEED_STATE, SEED_SEQ);
                fbm.init_generators();


                drawCylinder(state, fbm, state.depth / max_depth, state.depth / max_depth + 1 / max_depth,Vector2i(48, 48), positions, indices, temp);
            }

            int index = 0;
            for(auto t : temp) {
                float x_mapped = map->map(t.x(), index);
                texcoords.push_back(Vector2f(x_mapped, t.y()));
            }
        }


    }

    std::vector<Point3f> flower_anchors;
    std::vector<Point3f> bg_anchors;
    BoundingBox3f flower_bounds;
    int counter = 0;

protected:
    Transform trafo;
    pcg32 random;
    MultiDiffuseMap* map = nullptr;

};

class BGTree : public LGrammarConfig {

public:
    BGTree(pcg32& random, MultiDiffuseMap* map, float width_factor, float length_factor, float pitch_term, float yaw_term,
                   const Transform& trafo)
                : map(map),
                  random(random),
                  trafo(trafo),
                  LGrammarConfig(0.3f, 2.0f, width_factor, length_factor, pitch_term, yaw_term){}

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

        // do nothing
    }

    void controlYaw(TurtleState &state, char c) override {
        state.yaw += Warp::lineToLogistic(random.nextFloat(), 0.02);
    }

    void controlPitch(TurtleState &state, char c) override {
        state.pitch += Warp::lineToLogistic(random.nextFloat(), 0.02);
    }

    void controlLength(TurtleState &state, char c) override {
        state.length *= 1 + Warp::lineToLogistic(random.nextFloat(), 0.02);
        state.length = std::max(state.length, 0.1f);
    }

    void controlThickness(TurtleState &state, char c) override {
        state.out_thickness *= (1 + Warp::lineToLogistic(random.nextFloat(), 0.01));
        state.out_thickness = std::max(state.out_thickness, 0.02f);
    }

    int pickRule(char c, float thickness, float length, int depth) override {
        float sample = random.nextFloat();

        //Node =s Flower
        //Node =s Branch Depth [wl 30+ Node][wl 30- Node]; Branch Depth [wl 30+ 90> Node][wl 30- 90> Node]
        //Node =s Branch Depth [wl 30+ Node][wl 30- >>>> Node][wl 30- Node]

        if(c == 'F') {
            if(depth <= 3)  return pick(sample, 0.0, 0.1, 0.9); // always continue the trunk at low depth
            if(depth <= 5)  return pick(sample, 0.6, 0.0, 0.4);
            return                 pick(sample, 0.8, 0.0, 0.2);
        }

        if(c == 'K') {
            if(depth <= 4)  return pick(sample, 0.0,  0.4, 0.6);
            if(depth <= 6)  return pick(sample, 0.5,  0.1, 0.4);
            if(depth <= 8)  return pick(sample, 0.7,  0.1, 0.2);
            if(depth <= 12) return pick(sample, 0.9,  0.0, 0.1);
            return 0;
        }

        throw NoriException("Unhandled stochastic rule");
    }

    void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) override {

        controlFrame(state, c);

        // try reducing width much more after few depth

        if(state.random) {
            controlLength(state, c);
            controlPitch(state, c);
            controlYaw(state, c);
            controlThickness(state, c);
        }

        std::vector<Vector2f> temp;

        if (state.depth >= 3) {
            for(int i = 0; i < 3; i++) {
                float t = random.nextFloat() * state.length;
                float dx = 2 * random.nextFloat() - 1;
                float dy = 2 * random.nextFloat() - 1;
                float dz = 2 * random.nextFloat() - 2;

                Point3f pos = state.forward(t) + 0.0f * Vector3f(dx, dy, dz);
                drawMesh("assets/shape/sphere_low.obj", create_affine_matrix(0, 0, 0.2, pos), positions, indices, temp);
            }
            for(auto t : temp) {
                float x_mapped = map->map(t.x(), 1);
                texcoords.push_back(Vector2f(x_mapped, t.y()));
            }
            temp.clear();
        }

        if(c == 'G' || c == 'F') {

            drawCylinder(state, positions, indices, temp);

            int index = 0;
            for(auto t : temp) {
                float x_mapped = map->map(t.x(), index);
                texcoords.push_back(Vector2f(x_mapped, t.y()));
            }
            temp.clear();
        } else if(c == 'H') {
            drawMesh("assets/shape/sphere_low.obj", create_affine_matrix(0, 0, 0.3, state.p), positions, indices, temp);
            int index = 1;
            for(auto t : temp) {
                float x_mapped = map->map(t.x(), index);
                texcoords.push_back(Vector2f(x_mapped, t.y()));
            }
            temp.clear();
        }


    }

protected:
    Transform trafo;
    pcg32 random;
    MultiDiffuseMap* map = nullptr;

};

class BGTree2 : public LGrammarConfig {

public:
    BGTree2(pcg32& random, MultiDiffuseMap* map, float width_factor, float length_factor, float pitch_term, float yaw_term,
           const Transform& trafo)
            : map(map),
              random(random),
              trafo(trafo),
              LGrammarConfig(0.3f, 2.0f, width_factor, length_factor, pitch_term, yaw_term){}

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

        // do nothing
    }

    void controlYaw(TurtleState &state, char c) override {
        state.yaw += Warp::lineToLogistic(random.nextFloat(), 0.02);
    }

    void controlPitch(TurtleState &state, char c) override {
        state.pitch += Warp::lineToLogistic(random.nextFloat(), 0.02);
    }

    void controlLength(TurtleState &state, char c) override {
        state.length *= 1 + Warp::lineToLogistic(random.nextFloat(), 0.02);
        state.length = std::max(state.length, 0.1f);
    }

    void controlThickness(TurtleState &state, char c) override {
        state.out_thickness *= (1 + Warp::lineToLogistic(random.nextFloat(), 0.01));
        state.out_thickness = std::max(state.out_thickness, 0.02f);
    }

    int pickRule(char c, float thickness, float length, int depth) override {
        float sample = random.nextFloat();

        //Node =s FlowerBranch
        //Node =s Change direction
        //Node =s Split in two
        //Node =s Split in two, terminates one branch
        //Node =s Split in three

        if(c == 'K') {
            if(depth <= 4)  return pick(sample, 0.0, 0.0, 0.8, 0.0, 0.2, 0.0);
            if(depth <= 6)  return pick(sample, 0.0, 0.4, 0.3, 0.2, 0.0, 0.0);
            if(depth <= 8)  return pick(sample, 0.0, 0.1, 0.4, 0.4, 0.1, 0.0);
            if(depth <= 12) return pick(sample, 0.8, 0.0, 0.0, 0.2, 0.0, 0.0);
            return 0;
        }

        throw NoriException("Unhandled stochastic rule");
    }

    void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) override {

        controlFrame(state, c);

        // try reducing width much more after few depth

        if(state.random) {
            controlLength(state, c);
            controlPitch(state, c);
            controlYaw(state, c);
            controlThickness(state, c);
        }

        std::vector<Vector2f> temp;

        if (state.depth >= 3) {
            for(int i = 0; i < 3; i++) {
                float t = random.nextFloat() * state.length;
                float dx = 2 * random.nextFloat() - 1;
                float dy = 2 * random.nextFloat() - 1;
                float dz = 2 * random.nextFloat() - 2;

                Point3f pos = state.forward(t) + 0.1f * Vector3f(dx, dy, dz);
                drawMesh("assets/shape/sphere_low.obj", create_affine_matrix(0, 0, 0.01, pos), positions, indices, temp);

            }
            for(auto t : temp) {
                float x_mapped = map->map(t.x(), 1 + (int) (1 * random.nextFloat()));
                texcoords.push_back(Vector2f(x_mapped, t.y()));
            }
            temp.clear();
        }

        if(c == 'G' || c == 'F') {

            drawCylinder(state, positions, indices, temp);

            int index = 0;
            for(auto t : temp) {
                float x_mapped = map->map(t.x(), index);
                texcoords.push_back(Vector2f(x_mapped, t.y()));
            }
            temp.clear();
        } else if(c == 'H') {
            drawMesh("assets/shape/cube.obj", create_affine_matrix(0, 0, 0.5 * 0.3, state.p), positions, indices, temp);
            int index = 1;
            for(auto t : temp) {
                float x_mapped = map->map(t.x(), index);
                texcoords.push_back(Vector2f(x_mapped, t.y()));
            }
            temp.clear();
        }

    }

protected:
    Transform trafo;
    pcg32 random;
    MultiDiffuseMap* map = nullptr;

};

class RootConfig : public LGrammarConfig {

public:
    RootConfig(pcg32& random, MultiDiffuseMap* map, float width_factor, float length_factor, float pitch_term, float yaw_term,
                        const Transform& trafo)
                : map(map),
                  random(random),
                  trafo(trafo),
                  LGrammarConfig(2.0f, 3.5f, width_factor, length_factor, pitch_term, yaw_term){}

    int colorCount() override {
        return 1;
    }

    static int pick(float sample, float pa, float pb, float pc=0.f, float pd=0.f, float pe=0.f, float pf=0.f) {
        if(sample < pa) return 0;
        if(sample < pa + pb) return 1;
        if(sample < pa + pb + pc) return 2;
        if(sample < pa + pb + pc + pd) return 3;
        if(sample < pa + pb + pc + pd + pe) return 4;
        return 5;
    }

    int pickRule(char c, float thickness, float length, int depth) override {
        float sample = random.nextFloat();

        //Node =s Branch
        //Node =s Branch Depth 30- Node; Branch Depth 30+ Node; Branch Depth 30+ 90> Node; Branch Depth 30+ 90< Node
        //Node =s Branch Depth [wl 30+ Node][wl 30- Node]; Branch Depth [wl 30+ 90> Node][wl 30- 90> Node]

        if(c == 'K') {
            if(depth <= 2)  return pick(sample, 0.0, 0.0, 1.0);
            if(depth <= 4)  return pick(sample, 0.1, 0.1, 0.8);
            if(depth <= 6)  return pick(sample, 0.2, 0.2, 0.6);
            else            return pick(sample, 0.7, 0.3, 0.0);
        }

        throw NoriException("Unhandled stochastic rule");
    }

    void drawSegment(char c, TurtleState& state, std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) override {

        float max_depth = 7;

        if(state.depth >= max_depth) {
            drawCylinder(state, positions, indices, texcoords);
            return;
        }

        int BASE_RES_X  = 6;
        int BASE_RES_Z  = 6;
        uint64_t SEED_STATE = 3227124;
        uint64_t SEED_SEQ = 10951913;
        FBM fbm(BASE_RES_X, BASE_RES_Z, 5, 0.35, 2.0, SEED_STATE, SEED_SEQ);
        fbm.init_generators();


        drawCylinder(state, fbm, state.depth / max_depth, state.depth / max_depth + 1 / max_depth,Vector2i(64, 64), positions, indices, texcoords);
    }

protected:
    Transform trafo;
    pcg32 random;
    MultiDiffuseMap* map = nullptr;

};

NORI_NAMESPACE_END
