#include "Graph.h"
#include <algorithm>

Graph::Graph(std::vector<edges_t> &&_vertices) : vertices(_vertices), colors(vertices.size()) {

}

bool Graph::is_well_colored() const {
    // For all vertices...
    for (size_t idx = 0; idx < vertices.size(); idx++) {
        edges_t edges = vertices[idx];
        color_t from_color = colors[idx];
        // For all edges...
        for (const uint32_t &to_idx : edges) {
            color_t to_color = colors[to_idx];
            // Check that the color matches
            if (from_color == to_color)
                return false;
        }
    }
    return true;
}
