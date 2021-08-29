#include <algorithm>
#include <cmath>
#include <sstream>
#include <thread>
#include "../utils/PCVector.h"
#include "Parser.h"
#include "DimacsParser.h"

DimacsParser::DimacsParser(std::ifstream &_file, const std::string &filename)
        : file(std::move(_file)), fastparse_file(filename + ".fast"), num_threads(2) {
    if (!fastparse_file.is_open())
        throw std::runtime_error("Failed to open fastparse file for writing!");
}

Graph DimacsParser::parse() {
    std::string header;
    std::getline(file, header);
    // Some files begin with this string for some reason; read the next line if so
    if (header == "graph_for_greach")
        std::getline(file, header);
    num_vertices = std::stoul(header);

    // Contains the adjacency lists parsed from the file
    std::vector<edges_t> vertices = parse_lines();
    // A new vector where to merge vertices, so as not to risk creating the same edges twice
    std::vector<edges_t> merged_vertices = merge_adj_lists(vertices);

    serialize(merged_vertices, fastparse_file);
    // Ensure that the fastparse graph was written, so that crashes do not result in a malformed file
    fastparse_file.flush();

    return Graph(std::move(merged_vertices));
}

std::vector<uint32_t> DimacsParser::parse_numbers(const std::string &line) {
    std::vector<uint32_t> ret;

    std::istringstream line_str(line);
    std::string number_str;
    // std::getline reads line_str up to the next space and writes it into number_str
    // Skip the first token (containing the sequential node ID)
    std::getline(line_str, number_str, ' ');
    while (std::getline(line_str, number_str, ' ')) {
        if (number_str == "#")
            break;
        uint32_t number = std::stoul(number_str);
        ret.emplace_back(number);
    }
    return ret;
}

void DimacsParser::serialize(const std::vector<edges_t> &vertices, std::ostream &out) {
    // Start with the number of vertices (standardized to u32)
    serialize(static_cast<uint32_t>(vertices.size()), out);
    // Then serialize each vertex
    for (const edges_t &vertex : vertices)
        serialize(vertex, out);
}

void DimacsParser::serialize(const edges_t &vertex, std::ostream &out) {
    // Start with the number of neighbors
    serialize(static_cast<uint32_t>(vertex.size()), out);
    // Then serialize each neighbor
    for (const uint32_t &neighbor : vertex)
        serialize(neighbor, out);
}

void DimacsParser::serialize(uint32_t val, std::ostream &out) {
    out.write(reinterpret_cast<const char *>(&val), sizeof(uint32_t));
}

std::vector<edges_t> DimacsParser::parse_lines() {
    // Contains the adjacency lists parsed from the file
    std::vector<edges_t> vertices(num_vertices);

    // Empirically, the single-thread version performs better by ~30%.
    if (num_threads == 1) {
        std::string line;
        for (uint32_t i = 0; i < num_vertices; i++) {
            std::getline(file, line);
            vertices[i] = parse_numbers(line);
        }
    } else {
        using index_line_t = std::pair<std::vector<edges_t>::iterator, std::string>;
        PCVector<index_line_t> line_queue;
        line_queue.onReceive(num_threads, (void (*)(index_line_t)) [](index_line_t pair) {
            *pair.first = parse_numbers(pair.second);
        });
        std::string line;
        for (uint32_t i = 0; i < num_vertices; i++) {
            std::getline(file, line);
            line_queue.push(std::make_pair(vertices.begin() + i, line));
        }
        line_queue.stop();
        line_queue.join();
    }
    return vertices;
}

std::vector<edges_t> DimacsParser::merge_adj_lists(const std::vector<edges_t> &vertices) {
    std::vector<edges_t> merged_vertices(vertices);

    // Because DIMACS-10 only includes edges once (eg. 1->2 and not 2->1), we must merge the adjacency lists.
    // To do so in parallel, each thread can only write to a range of 1/N elements.
    // Each thread will iterate over all vertices and merge any relevant nodes.
    int items_per_thread = std::ceil(float(num_vertices)/float(num_threads));
    std::vector<std::thread> threads;
    for (int thread_idx = 0; thread_idx < num_threads; thread_idx++)
        threads.emplace_back([&, thread_idx, items_per_thread]() {
            // Range of lines that this thread is allowed to write: [range_lower, range_higher)
            int range_lower = items_per_thread * thread_idx;
            int range_higher = items_per_thread * (thread_idx + 1);

            for (const auto &vertex : vertices) {
                for (uint32_t edge : vertex) {
                    if (edge >= range_lower && edge < range_higher) {
                        merged_vertices[edge].push_back(edge);
                    }
                }
            }

            // Sorted vectors allow for efficient algorithms like std::set_intersection
            for (int pos = range_lower; pos < range_higher; pos++) {
                edges_t &edges = merged_vertices[pos];
                std::sort(edges.begin(), edges.end());
            }
        });

    for (auto &thread : threads)
        thread.join();

    return merged_vertices;
}
