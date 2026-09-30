#ifndef TAIFA_MINIMIZATION_MINIMIZER_H
#define TAIFA_MINIMIZATION_MINIMIZER_H

#include <fstream>

class AutomatonMinimizer {
public:
    virtual ~AutomatonMinimizer() = default;

    void Process(std::ifstream& input) {
        Parse(input);
        Partition initialPartition = GetInitialPartition();
        Partition finalPartition = RefinePartition(initialPartition);
        std::vector<MinimizedTransition> result = BuildResult(finalPartition);
        PrintResult(result);
    }

protected:
    AutomatonData m_data;

    void Parse(std::ifstream& input) {
        std::string line;
        while (std::getline(input, line)) {
            if (line.find("transitions:") != std::string::npos) {
                ParseStateLine(line);
            }
        }
        while (std::getline(input, line)) {
            ParseTransitionLine(line);
        }
    }

    Partition RefinePartition(Partition partition) {
        bool changed = true;
        while (changed) {
            changed = false;
            Partition newPartition;

            for (const auto& pair : partition) {
                const auto& group = pair.second;
                if (group.size() <= 1) {
                    newPartition[pair.first] = group;
                    continue;
                }

                std::map<std::vector<std::string>, std::vector<std::string>> subGroups;
                for (const auto& state : group) {
                    std::vector<std::string> nextGroupsVec;
                    for (const auto& input : m_data.Inputs) {
                        const auto& transition = m_data.TransitionMap.at(state).at(input);
                        const int stateGroup = GetStateGroup(transition.To, partition);
                        nextGroupsVec.push_back(std::to_string(stateGroup));
                    }
                    subGroups[nextGroupsVec].push_back(state);
                }

                if (subGroups.size() > 1) {
                    changed = true;
                }

                for (const auto& sub : subGroups) {
                    newPartition[sub.first] = sub.second;
                }
            }
            partition = newPartition;
        }
        return partition;
    }

    virtual void ParseStateLine(const std::string& line) = 0;
    virtual void ParseTransitionLine(const std::string& line) = 0;
    virtual Partition GetInitialPartition() = 0;
    virtual std::vector<MinimizedTransition> BuildResult(const Partition& partition) = 0;
    virtual void PrintResult(const std::vector<MinimizedTransition>& result) = 0;

private:
    int GetStateGroup(const std::string& state, const Partition& partition) {
        int i = 0;
        for (const auto& pair : partition) {
            if (std::find(pair.second.begin(), pair.second.end(), state) != pair.second.end()) {
                return i;
            }
            i++;
        }
        return -1;
    }
};

#endif //TAIFA_MINIMIZATION_MINIMIZER_H