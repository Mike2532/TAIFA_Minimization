#ifndef TAIFA_MINIMIZATION_MEALY_H
#define TAIFA_MINIMIZATION_MEALY_H
#include <iosfwd>
#include <iostream>
#include <sstream>
#include <string>

#include "Common.h"
#include "Minimizer.h"

class MealyMinimizer : public AutomatonMinimizer
{
protected:
    void ParseStateLine(const std::string& line) override {
        std::stringstream ss(line);
        std::string state, pipe, label;
        if (ss >> state >> pipe >> label) {
            m_data.States.insert(state);
        }
    }

    void ParseTransitionLine(const std::string& line) override {
        std::stringstream ss(line);
        std::string from, to, x, devider, y;
        if (ss >> from >> to >> x >> devider >> y) {
            m_data.States.insert(from);
            m_data.States.insert(to);
            m_data.Inputs.insert(x);
            m_data.TransitionMap[from][x] = {to, y};
        }
    }

    Partition GetInitialPartition() override {
        Partition partition;
        for (const auto& state : m_data.States) {
            std::vector<std::string> outVec;
            for (const auto& inp : m_data.Inputs) {
                outVec.push_back(m_data.TransitionMap.at(state).at(inp).Output);
            }
            partition[outVec].push_back(state);
        }
        return partition;
    }

    std::vector<MinimizedTransition> BuildResult(const Partition& partition) override {
        std::vector<MinimizedTransition> result;
        std::map<std::string, std::string> stateToNewName;
        int newNameId = 0;

        for (const auto& pair : partition) {
            std::string newName = "S" + std::to_string(newNameId++);
            for (const auto& state : pair.second) {
                stateToNewName[state] = newName;
            }
        }

        std::set<std::string> addedTransitions;
        for (const auto& pair : partition) {
            std::string repState = pair.second[0];
            std::string newFrom = stateToNewName.at(repState);

            for (const auto& inp : m_data.Inputs) {
                const auto& trans = m_data.TransitionMap.at(repState).at(inp);
                std::string newTo = stateToNewName.at(trans.To);
                std::string output = trans.Output;

                std::string key = newFrom + "|" + inp + "|" + newTo + "|" + output;
                if (addedTransitions.find(key) == addedTransitions.end()) {
                    addedTransitions.insert(key);
                    result.push_back({newFrom, newTo, inp, output});
                }
            }
        }
        return result;
    }

    void PrintResult(const std::vector<MinimizedTransition>& result) override {
        for (const auto& t : result) {
            std::cout << "  " << t.From << " " << t.To << " " << t.X << " / " << t.Y << "\n";
        }
    }
};

#endif //TAIFA_MINIMIZATION_MEALY_H