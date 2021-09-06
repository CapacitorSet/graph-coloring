#ifndef GRAPH_COLORING_PARSER_H
#define GRAPH_COLORING_PARSER_H

#include "../graph/Graph.h"

// Parser interface
class IParser {
public:
    virtual ~IParser() = default;
    virtual Graph parse() = 0;
};

class Parser {
    IParser *parser;
    bool serializable;
    std::string fast_filename;

public:
    double milliseconds;

    Parser(const std::string &filename);
    ~Parser();

    Graph parse();
};


#endif //GRAPH_COLORING_PARSER_H
