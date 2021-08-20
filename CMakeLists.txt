cmake_minimum_required(VERSION 3.16)
project(graph_coloring)

set(CMAKE_CXX_STANDARD 20)

add_executable(graph_coloring
        main.cpp
        graph/Graph.h graph/Graph.cpp
        parse/Parser.cpp parse/Parser.h
        parse/DimacsParser.cpp parse/DimacsParser.h
        parse/Dimacs10Parser.cpp parse/Dimacs10Parser.h
        parse/FastParser.cpp parse/FastParser.h
        solve/Solver.h
        solve/SequentialSolver.cpp solve/SequentialSolver.h
        solve/LubySolver.cpp solve/LubySolver.h
        solve/JonesSolver.cpp solve/JonesSolver.h
        solve/RandomPrioritySolver.cpp solve/RandomPrioritySolver.h
        solve/LDFsolver.cpp solve/LDFsolver.h
        solve/LDFsolverParallel.cpp solve/LDFsolverParallel.h
        benchmark/Benchmark.cpp benchmark/Benchmark.h
        benchmark/MemoryMonitor.cpp benchmark/MemoryMonitor.h
        utils/PCVector.h
        )
target_link_libraries(graph_coloring PRIVATE pthread)
