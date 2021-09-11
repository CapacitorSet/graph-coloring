#include <chrono>
#include <filesystem>
#include "Parser.h"
#include "FastParser.h"
#include "DimacsParser.h"
#include "Dimacs10Parser.h"
#include "Serializer.h"

Parser::Parser(const std::string &_path) {
    std::ifstream file(_path);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file.");

    auto path = std::filesystem::path(_path);

    if (path.extension() == ".fast") {
        serializable = false;
        parser = new FastParser(file);
    } else if (path.extension() == ".graph") {
        serializable = true;
        fast_filename = _path + ".fast";
        parser = new Dimacs10Parser(file, _path);
    } else if (path.extension() == ".gra") {
        serializable = true;
        fast_filename = _path + ".fast";
        parser = new DimacsParser(file, _path);
    } else {
        throw std::runtime_error("Unrecognized extension: ." + path.extension().string());
    }
    metadata.filename = path.filename().string();
}

Parser::~Parser() {
    delete parser;
}

Graph Parser::parse() {
    // Time the parsing
    auto t1 = std::chrono::high_resolution_clock::now();
    Graph g = parser->parse();
    auto t2 = std::chrono::high_resolution_clock::now();
    milliseconds = std::chrono::duration<double, std::milli>(t2 - t1).count();
    metadata.num_vertices = g.vertices.size();
    for (const auto &neighbors : g.vertices)
        metadata.num_edges += neighbors.size();
    // If we're not using FastParser, serialize the graph to a .fast file
    if (serializable) {
        std::ofstream fast_file(fast_filename);
        if (!fast_file.is_open())
            throw std::runtime_error("Failed to open FastParser file for writing.");
        Serializer s(g, fast_file);
    }
    return g;
}
