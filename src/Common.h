#ifndef TAIFA_MINIMIZATION_COMMON_H
#define TAIFA_MINIMIZATION_COMMON_H
#include <set>
#include <string>
#include <map>

struct MinimizedTransition {
    std::string From;
    std::string To;
    std::string X;
    std::string Y;
};

struct Transition {
    std::string To;
    std::string Output;
};

struct AutomatonData {
    std::set<std::string> States;
    std::set<std::string> Inputs;
    std::map<std::string, std::map<std::string, Transition>> TransitionMap;
};

using Partition = std::map<std::vector<std::string>, std::vector<std::string>>;

#endif //TAIFA_MINIMIZATION_COMMON_H