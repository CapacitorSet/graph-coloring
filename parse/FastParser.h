#ifndef GRAPH_COLORING_FASTPARSER1_H
#define GRAPH_COLORING_FASTPARSER1_H

#include <fstream>
#include "Parser.h"

class FastParser : public IParser {
    std::ifstream file;

    // Deserialize a single u32
    static uint32_t deserialize(std::istream &out);
public:
    FastParser(std::ifstream &);

    Graph parse();
};

#endif //GRAPH_COLORING_FASTPARSER1_H
