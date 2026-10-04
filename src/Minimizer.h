#ifndef TAIFA_MINIMIZATION_MINIMIZER_H
#define TAIFA_MINIMIZATION_MINIMIZER_H

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <stdexcept>

#include "Common.h"

inline std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

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
    std::string m_startState;

    void Parse(std::ifstream& input) {
        std::string line;
        bool inTransitions = false;

        while (std::getline(input, line)) {
            std::string cleanLine = trim(line);
            if (cleanLine.empty()) continue;

            if (cleanLine.find("transitions:") != std::string::npos) {
                inTransitions = true;
                continue;
            }

            if (cleanLine.find("start") != std::string::npos) {
                size_t colonPos = cleanLine.find(':');
                if (colonPos != std::string::npos) {
                    m_startState = trim(cleanLine.substr(colonPos + 1));
                }
                continue;
            }

            if (cleanLine.find("states:") != std::string::npos ||
                cleanLine.find("type:") != std::string::npos) {
                continue;
            }

            if (inTransitions) {
                ParseTransitionLine(cleanLine);
            } else {
                ParseStateLine(cleanLine);
            }
        }
    }

    Partition RefinePartition(Partition partition) {
        bool changed = true;
        while (changed) {
            changed = false;

            std::map<std::string, int> stateToGroupId;
            int gid = 0;
            for (const auto& pair : partition) {
                for (const auto& state : pair.second) {
                    stateToGroupId[state] = gid;
                }
                gid++;
            }

            Partition newPartition;

            for (const auto& pair : partition) {
                const auto& group = pair.second;
                if (group.size() <= 1) {
                    newPartition[pair.first] = group;
                    continue;
                }

                std::map<std::vector<int>, std::vector<std::string>> subGroups;
                for (const auto& state : group) {
                    std::vector<int> signature;
                    auto state_it = m_data.TransitionMap.find(state);

                    for (const auto& input : m_data.Inputs) {
                        int targetGroupId = -1;
                        if (state_it != m_data.TransitionMap.end()) {
                            auto inp_it = state_it->second.find(input);
                            if (inp_it != state_it->second.end()) {
                                auto target_it = stateToGroupId.find(inp_it->second.To);
                                if (target_it != stateToGroupId.end()) {
                                    targetGroupId = target_it->second;
                                }
                            }
                        }
                        signature.push_back(targetGroupId);
                    }
                    subGroups[signature].push_back(state);
                }

                if (subGroups.size() > 1) {
                    changed = true;
                }

                int subId = 0;
                for (const auto& sub : subGroups) {
                    std::vector<std::string> newKey = pair.first;
                    newKey.push_back("_sub" + std::to_string(subId++));
                    newPartition[newKey] = sub.second;
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
};

#endif //TAIFA_MINIMIZATION_MINIMIZER_H