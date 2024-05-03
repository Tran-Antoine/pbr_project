#pragma once

#include <core/common.h>
#include <core/vector.h>
#include <parser/lconfig.h>
#include <parser/turtle.h>
#include <stack>

NORI_NAMESPACE_BEGIN

class LSystemGrammar {

public:

    /**
     * List of symbols:
     *
     * F,G,H,K: Draw
     * [,]: Push/Restore turtle state
     * +,-: Rotate in the XY plane
     * <,>: Rotate in the XZ plane
     * w,W: Shrink/Increase width
     * l,L: Shrink/Increase length
     * r,d: Add/Remove noise feature to all values
     * D  : Increase depth marker
     * s  : Declare rule as stochastic (configuration decides the weightage)
     *
     * @param premise initial string
     * @param rules evolutionary rules
     */
    LSystemGrammar(std::string premise, std::vector<std::string> rules); //: premise(std::move(premise)), rules(std::move(rules)) {}

    std::vector<std::vector<std::string>> get_stochastic_rules();

    std::vector<std::string> get_deterministic_rules();

    std::string apply_det(const std::string& src, const std::string& rule);

    std::string apply_stoch(const std::string& src, const std::vector<std::string>& rule, LGrammarConfig& config);

    std::string evolve(int n, LGrammarConfig& config);

    static TurtleState simulate(const std::string& instructions, const LGrammarConfig& config) {

        std::stack<TurtleState> turtle_states;
        TurtleState current_state = {
                Vector3f(0.f), 0.f, M_PI / 2, Vector3f(0.f, 1.f, 0.f),
                config.get_initial_width(), config.get_initial_width(), config.get_length_factor()
        };
        TurtleState copy;

        for (auto instr: instructions) {
            switch (instr) {
                case '[':
                    copy = current_state;
                    turtle_states.push(copy);
                    break;
                case ']':
                    current_state = turtle_states.top();
                    turtle_states.pop();
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
                case 'D':
                    current_state.depth++;
                default:
                    break;
            }

        }
        return current_state;
    }

private:

    static bool isStochastic(std::string rule) {
        return rule.at(2) == 's';
    }
    std::string premise;
    std::vector<std::string> rules;

};


NORI_NAMESPACE_END
