#ifndef GRAPH_COLORING_GRAPH_H
#define GRAPH_COLORING_GRAPH_H

#include <cstdint>
#include <vector>

using edges_t = std::vector<uint32_t>;
using color_t = uint32_t;

class Graph {
    std::vector<edges_t> vertices;
    std::vector<color_t> colors;

    friend class SequentialSolver;

public:
    Graph(std::vector<edges_t> &&_vertices);

    bool is_well_colored() const;
    uint32_t count_colors() const;

    color_t color_of(uint32_t v) const;
    const edges_t & neighbors_of(uint32_t v) const;

    void clear_colors();
};

#endif //GRAPH_COLORING_GRAPH_H
