#include <sstream>
#include <algorithm>
#include "Parser.h"
#include "DimacsParser.h"

DimacsParser::DimacsParser(std::ifstream &_file, const std::string &filename)
        : file(std::move(_file)), fastparse_file(filename + ".fast") {
    if (!fastparse_file.is_open())
        throw std::runtime_error("Failed to open fastparse file for writing!");

}

// We do not need to worry about large copies: the compiler will move() the vector because of RVO
Graph DimacsParser::parse() {
    std::string header;
    std::getline(file, header);
    // Some files begin with this string for some reason; read the next line if so
    if (header == "graph_for_greach")
        std::getline(file, header);
    uint32_t numVertices = std::stoul(header);

    std::vector<edges_t> vertices(numVertices);
    std::string line;
    for (uint32_t i = 0; i < numVertices; i++) {
        std::getline(file, line);
        std::vector<uint32_t> edges = parse_numbers(line);
        // Unlike in DIMACS-10, in DIMACS the edges appear only once, so we must update the other end
        for (uint32_t edge : edges)
            vertices[edge].push_back(i);
        if (vertices[i].empty())
            vertices[i] = std::move(edges);
        else
            vertices[i].insert(vertices[i].cend(), edges.cbegin(), edges.cend());
    }

    // Sorted vectors allow for efficient algorithms like std::set_intersection
    for (edges_t &edges : vertices)
        std::sort(edges.begin(), edges.end());

    serialize(vertices, fastparse_file);
    // Ensure that the fastparse graph was written, so that crashes do not result in a malformed file
    fastparse_file.flush();

    return Graph(std::move(vertices));
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
