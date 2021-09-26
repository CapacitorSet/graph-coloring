#ifndef GRAPH_COLORING_GRAPH_H
#define GRAPH_COLORING_GRAPH_H

#include "../utils/span-lite.hpp"
#include <bitset>
#include <cstdint>
#include <vector>

using adjacency_vec_t = std::vector<uint32_t>;
using adjacency_list_t = nonstd::span<uint32_t>;
using color_t = uint32_t;

/* Graph implements the CSR data structure: we have a vector `neighbors` which holds all vertices' neighbors in sequence,
 * and a vector `neighbor_indices` which for each vertex points to the range in `vertices` where its neighbors lie.
 */
class Graph {
    std::vector<uint32_t> neighbors;
    std::vector<adjacency_list_t> neighbor_indices;
    std::vector<color_t> colors;

    friend class DeletableGraph;
    friend class Serializer;
    friend class Parser;

    friend class SequentialSolver;
    friend class LubySolver;
    friend class JonesSolver;
    friend class LDFSolver;
    friend class SDLSolver;
    friend class FVFSolver;
    friend class RandomSelectionSolver;

  public:
    Graph(const std::vector<adjacency_vec_t> &adj_list);

    bool is_well_colored() const;
    uint32_t count_colors() const;

    uint32_t num_vertices() const;
    color_t color_of(uint32_t v) const;
    adjacency_list_t neighbors_of(uint32_t v) const;
    uint32_t degree_of(uint32_t v) const;

    // Color vertex v with the smallest color that is not the same as a neighbor's, and return the color
    color_t color_with_smallest(uint32_t v);

    // Reset the graph for usage by another algorithm. Clears colors
    void clear();
};

class DeletableGraph {
    std::bitset<(1 << 24)> deleted; // Note that we support at most 2^24 nodes.

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
