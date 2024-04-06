#pragma once

#include <core/common.h>
#include <pcg32.h>
#include <stats/warp.h>

NORI_NAMESPACE_BEGIN

class LGrammarConfig {

public:

    virtual float randomizeYaw(float yaw) { return yaw; }
    virtual float randomizePitch(float pitch) { return pitch; }
    virtual float randomizeLength(float length) { return length; }
    virtual float randomizeThickness(float thickness) { return thickness; }
    virtual std::string specialRule(char c) { return std::string(1, c); }
};

class Config0 : public LGrammarConfig {

public:
    Config0(pcg32 random) : random(random) {}

    float randomizeYaw(float yaw) override {
        return yaw + M_PI / 4 * Warp::lineToLogistic(random.nextFloat(), 0.6);
    }

    float randomizePitch(float pitch) override {
        return pitch * (1 + 0.2f * Warp::lineToLogistic(random.nextFloat(), 0.2));
    }

    float randomizeLength(float length) override {
        return length * (1 + 0.3f * Warp::lineToLogistic(random.nextFloat(), 0.6));
    }
    float randomizeThickness(float thickness) override {
        return thickness * (1 + 0.1f * Warp::lineToLogistic(random.nextFloat(), 0.6));;
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
