#ifndef TAIFA_MINIMIZATION_MOORE_H
#define TAIFA_MINIMIZATION_MOORE_H

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <cstdio>

#include "Common.h"
#include "Minimizer.h"

class MooreMinimizer : public AutomatonMinimizer
{
protected:
    std::map<std::string, std::string> m_newStateToOutput;
    std::string m_newStartStateStr;

    void ParseStateLine(const std::string& line) override {
        std::stringstream ss(trim(line));
        std::string state, pipe, label, slash, output;
        if (ss >> state >> pipe >> label >> slash >> output) {
            state = trim(state);
            output = trim(output);
            m_data.States.insert(state);
            StateOutputs[state] = output;
        }
    }

    void ParseTransitionLine(const std::string& line) override {
        std::string cleanLine = trim(line);
        if (cleanLine.empty()) return;

        std::stringstream ss(cleanLine);
        std::string t1, t2, t3, t4;
        ss >> t1 >> t2 >> t3 >> t4;

        t1 = trim(t1); t2 = trim(t2); t3 = trim(t3); t4 = trim(t4);

        if (!t1.empty() && !t2.empty() && !t3.empty() && t4.empty()) {
            m_data.States.insert(t1);
            m_data.States.insert(t2);
            m_data.Inputs.insert(t3);
            m_data.TransitionMap[t1][t3] = {t2, ""};
        } else if (!t1.empty() && !t2.empty() && !t3.empty() && !t4.empty()) {
            m_data.States.insert(t1);
            m_data.States.insert(t3);
            m_data.Inputs.insert(t2);
            m_data.TransitionMap[t1][t2] = {t3, ""};
        } else {
            throw std::runtime_error("Не удалось распознать формат строки перехода Мура: '" + cleanLine + "'");
        }
    }

    Partition GetInitialPartition() override {
        Partition partition;
        for (const auto& state : m_data.States) {
            auto it = StateOutputs.find(state);
            if (it == StateOutputs.end()) {
                throw std::runtime_error("GetInitialPartition: Отсутствует определение выхода для состояния '" + state + "'");
            }
            partition[{it->second}].push_back(state);
        }
        return partition;
    }

    std::vector<MinimizedTransition> BuildResult(const Partition& partition) override {
        std::vector<MinimizedTransition> result;
        std::map<std::string, std::string> stateToNewName;
        int newNameId = 1;

        m_newStateToOutput.clear();
        m_newStartStateStr = "";

        for (const auto& pair : partition) {
            std::string newName = "S" + std::to_string(newNameId++);
            std::string output = StateOutputs[pair.second.front()];
            m_newStateToOutput[newName] = output;
            for (const auto& state : pair.second) {
                stateToNewName[state] = newName;
            }
        }

        if (!m_startState.empty() && stateToNewName.find(m_startState) != stateToNewName.end()) {
            std::string newStart = stateToNewName[m_startState];
            std::string out = StateOutputs[m_startState];
            m_newStartStateStr = newStart + "_" + out;
        }

        std::set<std::string> addedTransitions;
        for (const auto& pair : partition) {
            if (pair.second.empty()) continue;
            std::string repState = pair.second.front();

            auto from_it = stateToNewName.find(repState);
            if (from_it == stateToNewName.end()) {
                throw std::runtime_error("BuildResult: Состояние '" + repState + "' не найдено в stateToNewName.");
            }
            std::string newFrom = from_it->second;

            for (const auto& input : m_data.Inputs) {
                auto state_it = m_data.TransitionMap.find(repState);
                if (state_it == m_data.TransitionMap.end()) {
                    throw std::runtime_error("BuildResult: Состояние '" + repState + "' не найдено в TransitionMap.");
                }
                auto inp_it = state_it->second.find(input);
                if (inp_it == state_it->second.end()) {
                    throw std::runtime_error("BuildResult: Отсутствует переход для состояния '" + repState + "' по входу '" + input + "'");
                }
                const auto& trans = inp_it->second;

                auto to_it = stateToNewName.find(trans.To);
                if (to_it == stateToNewName.end()) {
                    throw std::runtime_error("КРИТИЧЕСКАЯ ОШИБКА: Целевое состояние '" + trans.To + "' отсутствует в stateToNewName!");
                }

                std::string newTo = to_it->second;
                std::string output = StateOutputs[repState];
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
        std::cout << "type: moore\n";
        if (!m_newStartStateStr.empty()) {
            std::cout << "start state: " << m_newStartStateStr << "\n\n";
        }

        std::cout << "states:\n";
        for (const auto& pair : m_newStateToOutput) {
            std::cout << pair.first << "_" << pair.second << " | " << pair.first << " / " << pair.second << "\n";
        }

        std::cout << "\ntransitions:\n";
        for (const auto& t : result) {
            std::string fromStr = t.From + "_" + m_newStateToOutput[t.From];
            std::string toStr = t.To + "_" + m_newStateToOutput[t.To];
            std::cout << fromStr << " " << toStr << " " << t.X << "\n";
        }
    }

private:
    std::map<std::string, std::string> StateOutputs;
};

#endif //TAIFA_MINIMIZATION_MOORE_H