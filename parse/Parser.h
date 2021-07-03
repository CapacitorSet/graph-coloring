#ifndef GRAPH_COLORING_PARSER_H
#define GRAPH_COLORING_PARSER_H

#include <string>
#include <vector>
#include <fstream>
#include "../graph/Graph.h"

class Parser {
    std::ifstream file;

    // Parse a line into a vector of numbers
    static std::vector<uint32_t> parse_numbers(const std::string &line, bool is_header);

public:
    Parser(const std::string &filename);

    Graph parse();
};


#endif //GRAPH_COLORING_PARSER_H
