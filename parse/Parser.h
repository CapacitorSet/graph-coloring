#ifndef GRAPH_COLORING_PARSER_H
#define GRAPH_COLORING_PARSER_H

#include <string>
#include <vector>
#include <fstream>
#include <variant>

#include "../graph/Graph.h"
#include "MetisParser.h"
#include "FastParser.h"

class Parser {
    // std::monostate is used when the variant is yet to be initialized (when the constructor is running).
    std::variant<std::monostate, MetisParser, FastParser> parser;

public:
    Parser(const std::string &filename);

    Graph parse();
};


#endif //GRAPH_COLORING_PARSER_H
