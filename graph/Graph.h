#ifndef GRAPH_COLORING_GRAPH_H
#define GRAPH_COLORING_GRAPH_H

#include <cstdint>
#include <vector>
#include <bitset>

using edges_t = std::vector<uint32_t>;
using color_t = uint32_t;

class Graph {
    std::vector<edges_t> vertices;
    std::vector<color_t> colors;

    friend class DeletableGraph;
    friend class Serializer;
    friend class Parser;

    friend class SequentialSolver;
    friend class LubySolver;
    friend class JonesSolver;
    friend class LDFsolver;
    friend class LDFsolverParallel;
    friend class RandomPrioritySolver;

public:
    Graph(std::vector<edges_t> &&_vertices);

    bool is_well_colored() const;
    uint32_t count_colors() const;

    color_t color_of(uint32_t v) const;
    const edges_t & neighbors_of(uint32_t v) const;
    uint32_t degree_of(uint32_t v) const;

    // Color vertex v with the smallest color that is not the same as a neighbor's, and return the color
    color_t color_with_smallest(uint32_t v);

    // Reset the graph for usage by another algorithm. Clears colors
    void clear();
};

class DeletableGraph {
    std::bitset<(1<<24)> deleted; // Note that we support at most 2^24 nodes.

public:
    const Graph &graph;

    DeletableGraph(const Graph &);

    void delete_vertex(uint32_t v);
    bool is_deleted(uint32_t v) const;
    [[nodiscard]] bool empty() const;

    // Reset the deleted set for usage by another algorithm
    void clear();
};

#endif //GRAPH_COLORING_GRAPH_H
