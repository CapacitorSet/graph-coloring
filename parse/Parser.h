#ifndef GRAPH_COLORING_PARSER_H
#define GRAPH_COLORING_PARSER_H

#include <string>
#include <vector>
#include <fstream>
#include <variant>

#include "../graph/Graph.h"
#include "DimacsParser.h"
#include "Dimacs10Parser.h"
#include "FastParser.h"

class Parser {
    // std::monostate is used when the variant is yet to be initialized (when the constructor is running).
    std::variant<std::monostate, DimacsParser, Dimacs10Parser, FastParser> parser;

public:
    Parser(const std::string &filename);

    Graph parse();
};


#endif //GRAPH_COLORING_PARSER_H
