#ifndef TAIFA_MINIMIZATION_MOORE_H
#define TAIFA_MINIMIZATION_MOORE_H
#include <iosfwd>
#include <iostream>
#include <sstream>
#include <string>

#include "Common.h"
#include "Minimizer.h"

class MooreMinimizer : public AutomatonMinimizer
{
protected:
    void ParseStateLine(const std::string& line) override {
        std::stringstream ss(line);
        std::string state, pipe, label, slash, output;
        if (ss >> state >> pipe >> label >> slash >> output) {
            m_data.States.insert(state);
            StateOutputs[state] = output;
        }
    }

    void ParseTransitionLine(const std::string& line) override {
        std::stringstream ss(line);
        std::string from, to, x;
        if (ss >> from >> to >> x) {
            m_data.States.insert(from);
            m_data.States.insert(to);
            m_data.Inputs.insert(x);
            m_data.TransitionMap[from][x] = {to, ""};
        }
    }

    Partition GetInitialPartition() override {
        Partition partition;
        for (const auto& state : m_data.States) {
            partition[{StateOutputs.at(state)}].push_back(state);
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
            std::string output = StateOutputs.at(repState);

            for (const auto& input : m_data.Inputs) {
                const auto& trans = m_data.TransitionMap.at(repState).at(input);
                std::string newTo = stateToNewName.at(trans.To);

                std::string key = newFrom + "|" + input + "|" + newTo + "|" + output;
                if (addedTransitions.find(key) == addedTransitions.end()) {
                    addedTransitions.insert(key);
                    result.push_back({newFrom, newTo, input, output});
                }
            }
        }
        return result;
    }

    void PrintResult(const std::vector<MinimizedTransition>& result) override {
        std::set<std::string> printedStates;
        for (const auto& t : result) {
            if (printedStates.find(t.From) == printedStates.end()) {
                std::cout << "  " << t.From << " / " << t.Y << std::endl;
                printedStates.insert(t.From);
            }
        }
        for (const auto& t : result) {
            std::cout << "  " << t.From << " " << t.To << " " << t.X << "\n";
        }
    }
private:
    std::map<std::string, std::string> StateOutputs;
};

#endif //TAIFA_MINIMIZATION_MOORE_H