#ifndef GRAPH_COLORING_DIMACSPARSER_H
#define GRAPH_COLORING_DIMACSPARSER_H

#include <fstream>
#include <vector>
#include "../graph/Graph.h"
#include "Parser.h"

class DimacsParser : public IParser {
    std::ifstream file;
    std::ofstream fastparse_file;
    int num_threads;
    uint32_t num_vertices;

    // Parse a line into a vector of numbers
    static std::vector<uint32_t> parse_numbers(const std::string &line);

    // Parse the adjacency lists, but do not merge them
    std::vector<edges_t> parse_lines();
    // Merge the adjacency lists
    std::vector<edges_t> merge_adj_lists(const std::vector<edges_t> &lists);

    // Serialize the parsed graph to a file for usage with FastParser
    static void serialize(const std::vector<edges_t> &vertices, std::ostream &out);
    // Serialize a single vertex
    static void serialize(const edges_t &vertex, std::ostream &out);
    // Serialize a single u32
    static void serialize(uint32_t, std::ostream &out);

public:
    DimacsParser(std::ifstream &, const std::string &filename);

    Graph parse();
};

#endif //GRAPH_COLORING_DIMACSPARSER_H
