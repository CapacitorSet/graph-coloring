#include "Graph.h"
#include <algorithm>
#include <cstddef>
#include <set>
#include <unordered_set>

Graph::Graph(std::vector<edges_t> &&_vertices) : vertices(std::move(_vertices)), colors(vertices.size()) {}

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

const edges_t &Graph::neighbors_of(uint32_t v) const {
    return vertices[v];
}

uint32_t Graph::degree_of(uint32_t v) const {
    return neighbors_of(v).size();
}

void Graph::clear() {
    std::fill(colors.begin(), colors.end(), 0);
    deleted.reset();
}

// It would be very slow to update the list of vertices to account for the removal of this vertex,
// so we just use a "deleted" bitset.
void Graph::remove_vertex(uint32_t v) {
    deleted.set(v);
    /*
    // Slow solution
    // Drop the current vertex from its neighbors' edges...
    for (uint32_t neighbor : vertices[v])
        vertices[neighbor].erase(std::find(vertices[neighbor].cbegin(), vertices[neighbor].cend(), v));
    // Drop the current vertex from this->vertices...
    vertices.erase(vertices.cbegin() + v);
    // And update existing neighbor IDs
    for (edges_t &edges : vertices) {
        for (uint32_t &neighbor : edges) {
            if (neighbor > v)
                neighbor--;
        }
    }
    */
}

bool Graph::is_deleted(uint32_t v) const {
    return deleted.test(v);
}

bool Graph::empty() const {
    return vertices.size() == deleted.count();
}

color_t Graph::color_with_smallest(uint32_t v) {
    std::set<color_t> neighbor_colors;
    for (const auto &neighbor : neighbors_of(v))
        neighbor_colors.emplace(color_of(neighbor));

    // Find smallest color not in the set of neighbor colors
    color_t smallest_color = 0;
    for (uint32_t neighbor_color : neighbor_colors)
        if (smallest_color != neighbor_color)
            break;
        else
            smallest_color++;
    colors[v] = smallest_color;

    return smallest_color;
}
