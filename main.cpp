#include <iostream>
#include "parse/Parser.h"
#include "solve/SequentialSolver.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Syntax: " << argv[0] << " <graph>" << std::endl;
        return 1;
    }
    Graph g = Parser(argv[1]).parse();
    Solver *s = new SequentialSolver();
    s->solve(g);
    if (g.is_well_colored()) {
        std::cout << "Graph colored correctly with " << std::to_string(g.count_colors()) << " colors!" << std::endl;
    } else {
        std::cout << "The graph was not colored." << std::endl;
    }
    return 0;
}
