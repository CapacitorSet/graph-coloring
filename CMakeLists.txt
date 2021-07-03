cmake_minimum_required(VERSION 3.20)
project(graph_coloring)

set(CMAKE_CXX_STANDARD 17)

add_executable(graph_coloring main.cpp parse/Parser.cpp parse/Parser.h graph/Graph.h graph/Graph.cpp graph/Graph.cpp graph/Graph.h solve/SequentialSolver.cpp solve/SequentialSolver.h solve/Solver.h)