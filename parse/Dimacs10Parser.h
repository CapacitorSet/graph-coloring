#ifndef GRAPH_COLORING_DIMACS10PARSER_H
#define GRAPH_COLORING_DIMACS10PARSER_H

#include <fstream>
#include <vector>
#include <thread>
#include "../graph/Graph.h"
#include "../utils/PCVector.h"

class Dimacs10Parser : public IParser {
    std::ifstream file;
    std::ofstream fastparse_file;

    using position_t = std::vector<edges_t>::iterator;
    // Store the string to be parsed and the position where to put it
    using message_t = std::pair<std::string, position_t>;
    PCVector<message_t> queue;
    std::vector<std::thread> threads;

    static void thread_function(PCVector<message_t> &);

    // Parse a line into a vector of numbers
    static std::vector<uint32_t> parse_numbers(const std::string &line, bool is_header = false);

    // Serialize the parsed graph to a file for usage with FastParser
    static void serialize(const std::vector<edges_t> &vertices, std::ostream &out);
    // Serialize a single vertex
    static void serialize(const edges_t &vertex, std::ostream &out);
    // Serialize a single u32
    static void serialize(uint32_t, std::ostream &out);

public:
    Dimacs10Parser(std::ifstream &, const std::string &filename);

    Graph parse();
};

#endif //GRAPH_COLORING_DIMACS10PARSER_H
