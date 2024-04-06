#pragma once

#include <core/common.h>
#include <pcg32.h>
#include <stats/warp.h>

NORI_NAMESPACE_BEGIN

class LGrammarConfig {

public:

    virtual float randomizeYaw(float yaw, char c) { return yaw; }
    virtual float randomizePitch(float pitch, char c) { return pitch; }
    virtual float randomizeLength(float length, char c) { return length; }
    virtual float randomizeThickness(float thickness, char c) { return thickness; }
    virtual int colorIndex(char c) { return 0; }
    virtual int colorCount() { return 0; }
    virtual std::string specialRule(char c) { return std::string(1, c); }
};

class Config0 : public LGrammarConfig {

public:
    Config0(pcg32 random) : random(random) {}

    float randomizeYaw(float yaw, char c) override {
        return yaw + M_PI / 4 * Warp::lineToLogistic(random.nextFloat(), 0.6);
    }

    float randomizePitch(float pitch, char c) override {
        return pitch * (1 + 0.2f * Warp::lineToLogistic(random.nextFloat(), 0.2));
    }

    float randomizeLength(float length, char c) override {
        //if(c == 'F') return 0.2f;

        return length * (1 + 0.3f * Warp::lineToLogistic(random.nextFloat(), 0.6));
    }
    float randomizeThickness(float thickness, char c) override {
        //if(c == 'F') return 0.12f;

        return thickness * (1 + 0.1f * Warp::lineToLogistic(random.nextFloat(), 0.6));;
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

    std::string specialRule(char c) override {
        float sample = random.nextFloat();

        if(c != 'F') {
            return std::string(1, c);
        }
        if(sample < 0.2) {
            return std::string("G");
        } else if(sample < 0.5) {
            return std::string("GF");
        } else if(sample < 0.8) {
            return std::string("G[wl+F]wl-F");
        } else {
            return std::string("G[wl+F][wlF]wl-F");
        }
    }

protected:
    pcg32 random;
};

NORI_NAMESPACE_END
