#include <algorithm>
#include <cmath>
#include <sstream>
#include <thread>
#include "../utils/PCVector.h"
#include "Parser.h"
#include "DimacsParser.h"
#include "../utils/RangeSplitter.h"

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
    RangeSplitter rs(num_vertices, num_threads);
    std::vector<std::thread> threads;
    for (int thread_idx = 0; thread_idx < num_threads; thread_idx++)
        threads.emplace_back([&, thread_idx, rs]() {
            // Range of lines that this thread is allowed to write: [range_lower, range_higher)
            int range_lower = rs.get_min(thread_idx),
                range_higher = rs.get_max(thread_idx);

            // If the edge 1->2 appears, we must create 2->1: vertices[destination].push_back(source).
            for (int source_id = 0; source_id < vertices.size(); source_id++) {
                auto &neighbors = vertices[source_id];
                for (uint32_t destination : neighbors)
                    if (destination >= range_lower && destination < range_higher)
                        merged_vertices[destination].push_back(source_id);
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
