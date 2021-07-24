#include "Parser.h"
#include "FastParser.h"

FastParser::FastParser(std::ifstream &_file) : file(std::move(_file)) {}

Graph FastParser::parse() {
    uint32_t num_vertices = deserialize(file);
    std::vector<edges_t> vertices(num_vertices);

    for (uint32_t i = 0; i < num_vertices; i++) {
        edges_t &edges = vertices[i];
        uint32_t num_edges = deserialize(file);
        edges.resize(num_edges);
        for (uint32_t j = 0; j < num_edges; j++)
            edges[j] = deserialize(file);
    }
    return Graph(std::move(vertices));
}

uint32_t FastParser::deserialize(std::istream &out) {
    uint32_t ret;
    out.read(reinterpret_cast<char *>(&ret), sizeof(uint32_t));
    return ret;
}
