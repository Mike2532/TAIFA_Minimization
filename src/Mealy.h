#ifndef TAIFA_MINIMIZATION_MEALY_H
#define TAIFA_MINIMIZATION_MEALY_H

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <cstdio>
#include <map>
#include <vector>

#include "Common.h"
#include "Minimizer.h"

class MealyMinimizer : public AutomatonMinimizer
{
protected:
    std::map<std::string, std::vector<std::string>> m_groupMapping;
    std::string m_newStartStateName;

    void ParseStateLine(const std::string& line) override {
        std::stringstream ss(trim(line));
        std::string state, pipe, label;
        if (ss >> state >> pipe >> label) {
            m_data.States.insert(trim(state));
        }
    }

    void ParseTransitionLine(const std::string& line) override {
        std::string cleanLine = trim(line);
        if (cleanLine.empty()) return;

        std::stringstream ss(cleanLine);
        std::string t1, t2, t3, t4, t5;
        ss >> t1 >> t2 >> t3 >> t4 >> t5;

        t1 = trim(t1); t2 = trim(t2); t3 = trim(t3); t4 = trim(t4); t5 = trim(t5);

        if (t4 == "/") {
            m_data.States.insert(t1);
            m_data.States.insert(t2);
            m_data.Inputs.insert(t3);
            m_data.TransitionMap[t1][t3] = {t2, t5};
        } else if (!t1.empty() && !t2.empty() && !t3.empty() && !t4.empty()) {
            m_data.States.insert(t1);
            m_data.States.insert(t3);
            m_data.Inputs.insert(t2);
            m_data.TransitionMap[t1][t2] = {t3, t4};
        } else {
            throw std::runtime_error("Не удалось распознать формат строки перехода: '" + cleanLine + "'");
        }
    }

    Partition GetInitialPartition() override {
        Partition partition;
        for (const auto& state : m_data.States) {
            std::vector<std::string> outVec;
            for (const auto& inp : m_data.Inputs) {
                auto state_it = m_data.TransitionMap.find(state);
                if (state_it == m_data.TransitionMap.end()) {
                    throw std::runtime_error("GetInitialPartition: Для состояния '" + state + "' нет ни одного перехода.");
                }
                auto inp_it = state_it->second.find(inp);
                if (inp_it == state_it->second.end()) {
                    throw std::runtime_error("GetInitialPartition: Отсутствует переход для состояния '" + state + "' по входу '" + inp + "'.");
                }
                outVec.push_back(inp_it->second.Output);
            }
            partition[outVec].push_back(state);
        }
        return partition;
    }

    std::vector<MinimizedTransition> BuildResult(const Partition& partition) override {
        std::vector<MinimizedTransition> result;
        std::map<std::string, std::string> stateToNewName;
        int newNameId = 1;

        m_groupMapping.clear();
        m_newStartStateName = "";

        for (const auto& pair : partition) {
            std::string newName = "S" + std::to_string(newNameId++);
            m_groupMapping[newName] = pair.second;
            for (const auto& state : pair.second) {
                stateToNewName[state] = newName;
            }
        }

        if (!m_startState.empty() && stateToNewName.find(m_startState) != stateToNewName.end()) {
            m_newStartStateName = stateToNewName[m_startState];
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

            for (const auto& inp : m_data.Inputs) {
                auto state_it = m_data.TransitionMap.find(repState);
                if (state_it == m_data.TransitionMap.end()) {
                    throw std::runtime_error("BuildResult: Состояние '" + repState + "' не найдено в TransitionMap.");
                }
                auto inp_it = state_it->second.find(inp);
                if (inp_it == state_it->second.end()) {
                    throw std::runtime_error("BuildResult: Отсутствует переход для состояния '" + repState + "' по входу '" + inp + "'.");
                }
                const auto& trans = inp_it->second;

                auto to_it = stateToNewName.find(trans.To);
                if (to_it == stateToNewName.end()) {
                    throw std::runtime_error("КРИТИЧЕСКАЯ ОШИБКА: Целевое состояние '" + trans.To + "' отсутствует в stateToNewName!");
                }

                std::string newTo = to_it->second;
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
        std::cout << "type: mealy\n";
        if (!m_newStartStateName.empty()) {
            std::cout << "start: " << m_newStartStateName << "\n\n";
        }

        std::cout << "transitions:\n";
        for (const auto& t : result) {
            std::cout << t.From << " " << t.To << " " << t.X << " / " << t.Y << "\n";
        }

        std::cout << "\n// Состав групп (Новое состояние <- Исходные состояния):\n";
        for (const auto& pair : m_groupMapping) {
            std::cout << "// " << pair.first << " <- ";
            for (size_t i = 0; i < pair.second.size(); ++i) {
                std::cout << pair.second[i];
                if (i < pair.second.size() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << "\n";
        }
    }
};

#endif //TAIFA_MINIMIZATION_MEALY_H