#include <iostream>
#include <fstream>
#include "src/Mealy.h"
#include "src/Minimizer.h"
#include "src/Moore.h"

int main() {
    try {
        std::ifstream input("../tests/N7_solutions/5_mealy_from_optimized_moor.txt");
        if (!input.is_open()) {
            throw std::runtime_error("Could not open file 'Input.txt'");
        }

        std::string line;
        if (!std::getline(input, line)) {
            throw std::runtime_error("Input file is empty");
        }

        std::unique_ptr<AutomatonMinimizer> minimizer;

        if (line == "type: mealy") {
            minimizer = std::make_unique<MealyMinimizer>();
        } else if (line == "type: moore") {
            minimizer = std::make_unique<MooreMinimizer>();
        } else {
            input.close();
            throw std::runtime_error("can not recogize input type");
        }

        minimizer->Process(input);
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
