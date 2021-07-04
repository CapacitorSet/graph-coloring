#include "Graph.h"
#include <cstddef>
#include <unordered_set>

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

uint32_t Graph::count_colors() const {
    // Construct an unordered set of colors on the fly and return the number of elements in it
    return std::unordered_set<color_t>(this->colors.cbegin(), this->colors.cend()).size();
}

color_t Graph::color_of(uint32_t v) const {
    return colors[v];
}

const edges_t& Graph::neighbors_of(uint32_t v) const {
    return vertices[v];
}

void Graph::clear_colors() {
    std::fill(colors.begin(), colors.end(), 0);
}
