#ifndef GRAPH_COLORING_DIMACSPARSER_H
#define GRAPH_COLORING_DIMACSPARSER_H

#include <fstream>
#include <vector>
#include "../graph/Graph.h"

class DimacsParser {
    std::ifstream file;
    std::ofstream fastparse_file;

    // Parse a line into a vector of numbers
    static std::vector<uint32_t> parse_numbers(const std::string &line);

    // Serialize the parsed graph to a file for usage with FastParser
    static void serialize(const std::vector<edges_t> &vertices, std::ostream &out);
    // Serialize a single vertex
    static void serialize(const edges_t &vertex, std::ostream &out);
    // Serialize a single u32
    static void serialize(uint32_t, std::ostream &out);

public:
    DimacsParser(std::ifstream &&, const std::string &filename);

    Graph parse();
};

#endif //GRAPH_COLORING_DIMACSPARSER_H
