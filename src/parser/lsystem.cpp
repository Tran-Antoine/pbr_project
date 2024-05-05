#pragma once

#include <core/common.h>
#include <parser/lsystem.h>
#include <core/mesh.h>
#include <core/timer.h>
#include <unordered_map>
#include <fstream>
#include <ImfRgbaFile.h>
#include <stack>
#include <pcg32.h>
#include <parser/lconfig.h>

NORI_NAMESPACE_BEGIN

LSystemGrammar::LSystemGrammar(std::string premise, std::vector<std::string> rules, pcg32& random) 
    : premise(std::move(premise)), rules(std::move(rules)), random(random) {}

std::vector<std::vector<std::string>> LSystemGrammar::get_stochastic_rules() {
    std::vector<std::vector<std::string>> stochastic_rules;
    for(const auto& r : rules) {
        if(!isStochastic(r)) {
            continue;
        }
        bool found = false;
        for(auto& slist : stochastic_rules) {
            if(slist[0].at(0) == r.at(0)) {
                slist.push_back(r);
                found = true;
                break;
            }
        }
        if(!found){
            stochastic_rules.push_back(std::vector<std::string>{r});
        }
    }
    return stochastic_rules;
}

std::vector<std::string> LSystemGrammar::get_deterministic_rules() {
    std::vector<std::string> det_rules;
    for(const auto& r : rules) {
        if(!isStochastic(r)) {
            det_rules.push_back(r);
        }
    }
    return det_rules;
}

std::string LSystemGrammar::apply_det(const std::string& src, const std::string& rule) {

    std::string chosen = pick_uniformly(rule.substr(2));

    std::string out;
    for(auto c : src) {
        if(rule.at(0) == c) {
            out += chosen;
        } else {
            out += c;
        }
    }
    return out;
}



std::string LSystemGrammar::apply_stoch(const std::string& src, const std::vector<std::string>& rule, LGrammarConfig& config) {

    std::string out;
    char target_char = rule[0].at(0);

    for(auto c : src) {
        if(c == target_char) {
            TurtleState state = simulate(out, config);
            const std::string& single_rule = rule[config.pickRule(c, state.in_thickness, state.length, state.depth)];
            std::string processed_rule = pick_uniformly(single_rule.substr(3));
            out += processed_rule;
        } else {
            out += c;
        }
    }

    return out;
}

std::string LSystemGrammar::evolve(int n, LGrammarConfig& config) {

    std::vector<std::string> det_rules = get_deterministic_rules();
    std::vector<std::vector<std::string>> stoch_rules = get_stochastic_rules();

    std::string current = premise;
    for(int i = 0; i < n; i++) {

        for(const auto& rule : stoch_rules) {
            current = apply_stoch(current, rule, config);
        }

        for(const auto& rule : det_rules) {
            current = apply_det(current, rule);
        }
    }
    return current;
}

class LSystemMesh : public Mesh {

public:
    explicit LSystemMesh(const PropertyList& propList) {

        config_file = propList.getString("config");

        if(config_file.empty()) {
            // manually fetch premise + rules
            premise = propList.getString("premise");
            for(int i = 0; i < 30; i++) {
                std::string tagName = "rule" + std::to_string(i);
                std::string rule = propList.getString(tagName, "");
                if(rule.empty()) break;

                rules.push_back(rule);
            }
        }

        n = propList.getInteger("evolutions");
        random.seed((uint64_t) propList.getFloat("seed", 11111));

        width_factor = propList.getFloat("width_factor", 0.7f);
        length_factor = propList.getFloat("length_factor", 0.7f);
        pitch_term = degToRad(propList.getFloat("pitch_term", 45.f));
        yaw_term = degToRad(propList.getFloat("yaw_term", 45.f));
        bump_increase_factor = propList.getFloat("bump_accentuate", 1.f);
        trafo = propList.getTransform("toWorld", Transform());

    }

    void activate() override {
        auto config = Config5(random, map, width_factor, length_factor, pitch_term, yaw_term);
        std::string mesh_string = (config_file.empty()
                ? LSystemGrammar(premise, rules, random)
                : LSystemGrammar::fromConfig(random, config_file))
                    .evolve(n, config);

        Timer timer;
        std::vector<Vector3f>   positions;
        std::vector<uint32_t>   indices;
        std::vector<Vector2f>   texcoords;

        std::cout << mesh_string << std::endl;

        drawLSystem(mesh_string, config, positions, indices, texcoords);

        for(const auto& p : positions) {
            m_bbox.expandBy(p);
        }

        m_F.resize(3, indices.size()/3);

        for (uint32_t i=0; i<indices.size()/3; ++i) {
            m_F.col(i).x() = indices[3*i];
            m_F.col(i).y() = indices[3*i+1];
            m_F.col(i).z() = indices[3*i+2];
        }

        m_V.resize(3, positions.size());
        for (uint32_t i = 0; i < positions.size(); ++i) {
            m_V.col(i) = trafo * positions[i];
        }

        if (!texcoords.empty()) {
            m_UV.resize(2, positions.size());
            for (uint32_t i=0; i<texcoords.size(); ++i) {
                m_UV.col(i) = texcoords[i];
            }
        }

        cout << "done. (V=" << m_V.cols() << ", F=" << m_F.cols() << ", took "
             << timer.elapsedString() << " and "
             << memString(m_F.size() * sizeof(uint32_t) +
                          sizeof(float) * (m_V.size() + m_N.size() + m_UV.size()))
             << ")" << endl;
    }

    void addChild(nori::NoriObject *obj) override {
        switch (obj->getClassType()) {
            case NoriObject::EDiffuseMap:
                map = static_cast<MultiDiffuseMap *>(obj);
                break;
            default:
                Mesh::addChild(obj);
                break;
        }
    }

protected:

    pcg32 random;
    float width_factor, length_factor, pitch_term, yaw_term;
    Transform trafo;
    int n;
    std::string config_file;
    std::string premise;
    std::vector<std::string> rules;

    MultiDiffuseMap* map = nullptr;

    void drawLSystem(const std::string &seq, LGrammarConfig& config,
                     std::vector<Vector3f> &positions, std::vector<uint32_t> &indices, std::vector<Vector2f> &texcoords) {

        std::stack<TurtleState> turtle_states;

        TurtleState current_state = {
                Vector3f(0.f), 0.f, M_PI / 2, Vector3f(0.f, 1.f, 0.f),
                config.get_initial_width(), config.get_initial_width(), config.get_length_factor()
        };

        TurtleState copy;

        std::string instructions = seq;
        int index = 0;
        while(index < instructions.length()) {
            char instr = instructions.at(index);
            switch(instr) {
                case '[':
                    copy = current_state;
                    turtle_states.push(copy);
                    break;
                case ']':
                    current_state = turtle_states.top();
                    turtle_states.pop();
                    break;
                case 'F':
                case 'G':
                case 'H':
                case 'K':
                    config.drawSegment(instr, current_state, positions, indices, texcoords);
                    break;
                case '+':
                    current_state.pitch += config.get_pitch_term();
                    break;
                case '-':
                    current_state.pitch -= config.get_pitch_term();
                    break;
                case '>':
                    current_state.yaw += config.get_yaw_term();
                    break;
                case '<':
                    current_state.yaw -= config.get_yaw_term();
                    break;
                case 'w':
                    current_state.out_thickness *= config.get_width_factor();
                    break;
                case 'W':
                    current_state.out_thickness *= 1 / config.get_width_factor();
                    break;
                case 'l':
                    current_state.length *= config.get_length_factor();
                    break;
                case 'L':
                    current_state.length *= 1 / config.get_length_factor();
                    break;
                case 'r':
                    current_state.random = true;
                    break;
                case 'd':
                    current_state.random = false;
                case 'D':
                    current_state.depth++;
                default: break;
            }

            index++;
        }
    }
};

NORI_REGISTER_CLASS(LSystemMesh, "lsystem");
NORI_NAMESPACE_END