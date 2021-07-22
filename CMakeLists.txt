cmake_minimum_required(VERSION 3.20)
project(graph_coloring)

set(CMAKE_CXX_STANDARD 17)

add_executable(graph_coloring
        main.cpp
        graph/Graph.h graph/Graph.cpp
        parse/Parser.cpp parse/Parser.h
        parse/Dimacs10Parser.cpp parse/Dimacs10Parser.h
        parse/FastParser.cpp parse/FastParser.h
        solve/Solver.h
        solve/SequentialSolver.cpp solve/SequentialSolver.h
        solve/LubySolver.cpp solve/LubySolver.h
        solve/JonesSolver.cpp solve/JonesSolver.h
        benchmark/Benchmark.cpp benchmark/Benchmark.h
)
target_link_libraries(graph_coloring PRIVATE pthread)
