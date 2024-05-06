#pragma once

#include <core/common.h>
#include <core/vector.h>
#include <parser/lconfig.h>
#include <parser/turtle.h>
#include <stack>
#include <sstream> 
#include <iostream> 
#include <pcg32.h>
#include <fstream>

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
     * ;  : Add multiple uniformly distributed options
     * s  : Declare rule as stochastic (configuration decides the weightage)
     *
     * @param premise initial string
     * @param rules evolutionary rules
     */
    LSystemGrammar(std::string premise, std::vector<std::string> rules, pcg32& random); //: premise(std::move(premise)), rules(std::move(rules)) {}

    std::vector<std::vector<std::string>> get_stochastic_rules();

    std::vector<std::string> get_deterministic_rules();

    std::string apply_det(const std::string& src, const std::string& rule);

    std::string apply_stoch(const std::string& src, const std::vector<std::string>& rule, LGrammarConfig& config);

    std::string evolve(int n, LGrammarConfig& config);

    static TurtleState simulate(const std::string& instructions, const LGrammarConfig& config) {

        std::stack<TurtleState> turtle_states;
        TurtleState current_state = {
                Vector3f(0.f), Vector3f(0, 1, 0), 0.f, M_PI / 2, Vector3f(0.f, 1.f, 0.f),
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

    static std::string subst(const std::string& src, std::map<std::string, std::string, std::function<bool(const std::string&, const std::string&)>>& substs) {
        std::string out = src;
        for (const auto &pair : substs) {
            std::string key = pair.first;
            std::string value = pair.second;
            size_t pos = 0;

            // Replace all occurrences of the current key
            while ((pos = out.find(key, pos)) != std::string::npos) {
                out.replace(pos, key.length(), value);
                pos += value.length();
            }
        }
        return out;
    }

    static void strip(std::string& s) {
        s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
        s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
        s.erase(remove_if(s.begin(), s.end(), isspace), s.end());
    }

    static bool split_statement(const std::string& src, std::string& lhs, std::string& rhs) {
        std::istringstream iss(src);
        if (std::getline(iss, lhs, '=') && std::getline(iss, rhs)) {
            return true;
        }
        return false;
    }

    static LSystemGrammar fromConfig(pcg32& random, const std::string& filename) {
        
        // Open the file in read-only mode
        std::ifstream file(filename);

        if (!file.is_open()) {
            std::cerr << "Error opening file at " << filename << std::endl;
            throw NoriException("Crash due to IO error");
        }

        std::string premise;
        std::vector<std::string> rules;
        auto comp = [](const std::string& a, const std::string& b) {
            if(a.size() != b.size()) {
                return a.size() > b.size();
            }
            return a < b;
        };
        std::map<std::string, std::string, std::function<bool(const std::string&, const std::string&)>> sugar_map(comp);
        bool reading_sugar = true;

        std::string line;
        // Read and print each line of the file
        while (std::getline(file, line)) {

            if (line.find("#RULES") == 0) {
                reading_sugar = false;
                continue;
            }


            std::string lhs, rhs;
            bool valid_line = split_statement(line, lhs, rhs);

            if(!valid_line) {
                // Skip lines that aren't a statement
                continue;
            }

            if (reading_sugar) {
                strip(lhs); strip(rhs);
                sugar_map[lhs] = rhs;
                continue;
            }

            if(line.find("Premise") == 0) {
                if(!premise.empty()) {
                    throw NoriException("Multiple premises defined");
                }
                std::string premise_, unused;
                if(!split_statement(line, unused, premise_)) {
                    throw NoriException("Syntax error");
                }
                premise = subst(premise_, sugar_map);
                strip(premise);
                continue;
            }

            std::string left_rule  = subst(lhs, sugar_map); strip(left_rule);
            std::string right_rule = subst(rhs, sugar_map); strip(right_rule);

            rules.push_back(left_rule + "=" + right_rule);
        }

        // Close the file
        file.close();

        return LSystemGrammar(premise, rules, random);
    }

private:

    static bool isStochastic(std::string rule) {
        return rule.at(2) == 's';
    }

    static std::vector<std::string> split(std::string in) {
        // Create a stringstream object with the input string 
        std::stringstream ss(in);
    
        // Tokenize the input string by comma delimiter 
        std::string token; 
        std::vector<std::string> tokens; 
        char delimiter = ';'; 
    
        while (getline(ss, token, delimiter)) { 
            tokens.push_back(token); 
        }

        return tokens; 
    }

    inline std::string pick_uniformly(std::string rule) {
        auto options = split(rule);

        // avoid calculating a random sample for nothing
        if(options.size() == 1) {
            return options[0];
        }

        int index = (int) (random.nextFloat() * options.size());
        return options[index];
    }

    std::string premise;
    std::vector<std::string> rules;
    pcg32 random;
};


NORI_NAMESPACE_END
