#ifndef GRAPH_COLORING_DIMACS10PARSER_H
#define GRAPH_COLORING_DIMACS10PARSER_H

#include "../graph/Graph.h"
#include "Parser.h"
#include "../utils/PCVector.h"
#include <fstream>
#include <thread>
#include <vector>

class Dimacs10Parser : public IParser {
    std::ifstream file;
    std::ofstream fastparse_file;

    using position_t = std::vector<adjacency_vec_t>::iterator;
    // Store the string to be parsed and the position where to put it
    using message_t = std::pair<std::string, position_t>;
    PCVector<message_t> queue;

    // Parse a line into a vector of numbers
    static std::vector<uint32_t> parse_numbers(const std::string &line, bool is_header = false);

  public:
    Dimacs10Parser(std::ifstream &, const std::string &filename);

    Graph parse();
};

#endif //GRAPH_COLORING_DIMACS10PARSER_H
