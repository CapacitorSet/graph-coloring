#include <chrono>
#include "Parser.h"
#include "FastParser.h"
#include "DimacsParser.h"
#include "Dimacs10Parser.h"
#include "Serializer.h"

Parser::Parser(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file.");

    if (filename.find(".fast") != std::string::npos) {
        serializable = false;
        parser = new FastParser(file);
    } else if (filename.find(".graph") != std::string::npos) {
        serializable = true;
        fast_filename = filename + ".fast";
        parser = new Dimacs10Parser(file, filename);
    } else if (filename.find(".gra") != std::string::npos) {
        serializable = true;
        fast_filename = filename + ".fast";
        parser = new DimacsParser(file, filename);
    } else {
        throw std::runtime_error("Unrecognized file format");
    }
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
    // If we're not using FastParser, serialize the graph to a .fast file
    if (serializable) {
        std::ofstream fast_file(fast_filename);
        if (!fast_file.is_open())
            throw std::runtime_error("Failed to open FastParser file for writing.");
        Serializer s(g, fast_file);
    }
    return g;
}
