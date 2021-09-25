#include "Dimacs10Parser.h"
#include "Parser.h"
#include <algorithm>
#include <sstream>

Dimacs10Parser::Dimacs10Parser(std::ifstream &_file, const std::string &filename)
    : file(std::move(_file)), fastparse_file(filename + ".fast") {
    if (!fastparse_file.is_open())
        throw std::runtime_error("Failed to open fastparse file for writing!");
}

// We do not need to worry about large copies: the compiler will move() the vector because of RVO
Graph Dimacs10Parser::parse() {
    std::string header;
    std::getline(file, header);
    uint32_t numVertices;

    std::vector<uint32_t> header_vals = parse_numbers(header, true);
    switch (header_vals.size()) {
    case 2:
        numVertices = header_vals[0];
        // numEdges = header_vals[1];
        break;
    default:
        throw std::runtime_error("Unexpected header size");
    }

    queue.onReceive(
        4, (void (*)(message_t))[](message_t message) {
            // Parse the line and put it into the iterator
            // Note that no locking is required: we always access different positions in the vector
            *message.second = parse_numbers(message.first);
        });

    std::vector<edges_t> vertices(numVertices);
    std::string line;
    for (uint32_t i = 0; i < numVertices; i++) {
        std::getline(file, line);
        queue.push(std::make_pair(line, vertices.begin() + i));
    }

    queue.stop();
    queue.join();

    return Graph(std::move(vertices));
}

std::vector<uint32_t> Dimacs10Parser::parse_numbers(const std::string &line, bool is_header) {
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