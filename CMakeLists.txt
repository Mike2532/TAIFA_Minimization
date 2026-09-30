cmake_minimum_required(VERSION 4.0)
project(TAIFA_Minimization)

set(CMAKE_CXX_STANDARD 20)

add_executable(TAIFA_Minimization main.cpp
        src/Common.h
        src/Minimizer.h
        src/Moore.h
        src/Mealy.h)
