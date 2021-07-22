#include <sstream>
#include <algorithm>
#include "Parser.h"
#include "MetisParser.h"

MetisParser::MetisParser(std::ifstream &&_file, const std::string &filename)
        : file(std::move(_file)), fastparse_file(filename + ".fast") {
    if (!fastparse_file.is_open())
        throw "Failed to open fastparse file for writing!";

}

// We do not need to worry about large copies: the compiler will move() the vector because of RVO
Graph MetisParser::parse() {
    std::string header;
    std::getline(file, header);
    uint32_t numVertices, numEdges;

    std::vector<uint32_t> header_vals = parse_numbers(header, true);
    // std::cout << "Header: " << std::to_string(header_vals.size()) << " values read." << std::endl;
    switch (header_vals.size()) {
        case 2:
            numVertices = header_vals[0];
            numEdges = header_vals[1];
            break;
        default:
            throw "Unexpected header size";
    }

    std::vector<edges_t> vertices;
    vertices.resize(numVertices);
    std::string line;
    for (uint32_t i = 0; i < numVertices; i++) {
        std::getline(file, line);
        std::vector<uint32_t> edges = parse_numbers(line);
        vertices[i] = std::move(edges);
    }

    serialize(vertices, fastparse_file);

    return Graph(std::move(vertices));
}

std::vector<uint32_t> MetisParser::parse_numbers(const std::string &line, bool is_header) {
    std::vector<uint32_t> ret;
    std::istringstream line_str(line);
    std::string number_str;
    // std::getline reads line_str up to the next space and writes it into number_str
    while (std::getline(line_str, number_str, ' ')) {
        uint32_t number = std::stoul(number_str);
        if (!is_header)
            number -= 1; // In the Metis format, vertices start from 1
        ret.emplace_back(number);
    }
    // Sorted vectors allows for efficient algorithms like std::set_intersection
    std::sort(ret.begin(), ret.end());
    return ret;
}

void MetisParser::serialize(const std::vector<edges_t> &vertices, std::ostream &out) {
    // Start with the number of vertices (standardized to u32)
    serialize(static_cast<uint32_t>(vertices.size()), out);
    // Then serialize each vertex
    for (const edges_t &vertex : vertices)
        serialize(vertex, out);
}

void MetisParser::serialize(const edges_t &vertex, std::ostream &out) {
    // Start with the number of neighbors
    serialize(static_cast<uint32_t>(vertex.size()), out);
    // Then serialize each neighbor
    for (const uint32_t &neighbor : vertex)
        serialize(neighbor, out);
}

void MetisParser::serialize(uint32_t val, std::ostream &out) {
    out.write(reinterpret_cast<const char *>(&val), sizeof(uint32_t));
}
