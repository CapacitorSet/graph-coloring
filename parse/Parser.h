#ifndef GRAPH_COLORING_PARSER_H
#define GRAPH_COLORING_PARSER_H

#include <string>
#include <vector>
#include <fstream>
#include <variant>
#include "../graph/Graph.h"

class MetisParser {
    std::ifstream file;
    std::ofstream fastparse_file;

    // Parse a line into a vector of numbers
    static std::vector<uint32_t> parse_numbers(const std::string &line, bool is_header = false);

    // Serialize the parsed graph to a file for usage with FastParser
    static void serialize(const std::vector<edges_t> &vertices, std::ostream &out);
    // Serialize a single vertex
    static void serialize(const edges_t &vertex, std::ostream &out);
    // Serialize a single u32
    static void serialize(uint32_t, std::ostream &out);

public:
    MetisParser(std::ifstream &&, const std::string &filename);

    Graph parse();
};

class FastParser {
    std::ifstream file;

    // Deserialize a single u32
    static uint32_t deserialize(std::istream &out);
public:
    FastParser(std::ifstream &&);

    Graph parse();
};

class Parser {
    // std::monostate is used when the variant is yet to be initialized (when the constructor is running).
    std::variant<std::monostate, MetisParser, FastParser> parser;

public:
    Parser(const std::string &filename);

    Graph parse();
};


#endif //GRAPH_COLORING_PARSER_H
