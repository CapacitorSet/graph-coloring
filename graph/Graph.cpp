#include "Graph.h"
#include <algorithm>
#include <cstddef>
#include <set>
#include <unordered_set>

Graph::Graph(const std::vector<std::vector<uint32_t>> &adj_list) : colors(adj_list.size()), neighbor_indices(adj_list.size()) {
    for (size_t idx = 0; idx < adj_list.size(); idx++) {
        const auto &list = adj_list[idx];
        auto begin_it = neighbors.insert(neighbors.end(), list.begin(), list.end());
        // neighbor_indices[idx] = nonstd::span<uint32_t>(begin_it, list.size());
    }
    auto begin_it = neighbors.begin();
    for (size_t idx = 0; idx < adj_list.size(); idx++) {
        const auto &list = adj_list[idx];
        auto end_it = begin_it + list.size();
        neighbor_indices[idx] = nonstd::span<uint32_t>(begin_it, end_it);
        begin_it = end_it;
    }
}

bool Graph::is_well_colored() const {
    // For all vertices...
    for (size_t idx = 0; idx < neighbor_indices.size(); idx++) {
        color_t from_color = color_of(idx);
        // For all edges...
        for (const uint32_t &to_idx : neighbors_of(idx)) {
            color_t to_color = color_of(to_idx);
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

uint32_t Graph::num_vertices() const {
    return neighbor_indices.size();
}

color_t Graph::color_of(uint32_t v) const {
    return colors[v];
}

adjacency_list_t Graph::neighbors_of(uint32_t v) const {
    return neighbor_indices[v];
}

uint32_t Graph::degree_of(uint32_t v) const {
    return neighbors_of(v).size();
}

void Graph::clear() {
    std::fill(colors.begin(), colors.end(), 0);
}

/* It would be very slow to update the list of vertices to account for the removal of this vertex,
 * so we just use a "deleted" bitset in DeletableGraph.
 * Slow implementation:
void Graph::remove_vertex(uint32_t v) {
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
}
*/

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

DeletableGraph::DeletableGraph(const Graph &graph) : graph(graph) {}

void DeletableGraph::delete_vertex(uint32_t v) {
    deleted.set(v);
}

bool DeletableGraph::is_deleted(uint32_t v) const {
    return deleted.test(v);
}

bool DeletableGraph::empty() const {
    return graph.num_vertices() == deleted.count();
}

void DeletableGraph::clear() {
    deleted.reset();
}