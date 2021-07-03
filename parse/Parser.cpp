#include <iostream>
#include <sstream>
#include "Parser.h"

Parser::Parser(const std::string &filename) : file(filename) {
    if (!file.is_open())
        throw "Failed to open file.";
}

std::vector<uint32_t> Parser::parse_numbers(const std::string &line, bool is_header = false) {
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
    return ret;
}

// We do not need to worry about large copies: the compiler will move() the vector because of RVO
Graph Parser::parse() {
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

    return Graph(std::move(vertices));
}
