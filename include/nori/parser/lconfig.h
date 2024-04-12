#pragma once

#include <core/common.h>
#include <parser/lsystem.h>
#include <parser/turtle.h>
#include <pcg32.h>
#include <stats/warp.h>

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
        Config1(pcg32& random, float width_factor, float length_factor, float pitch_term, float yaw_term) : random(random),
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


NORI_NAMESPACE_END
