#include <chrono>
#include "Parser.h"
#include "FastParser.h"
#include "DimacsParser.h"
#include "Dimacs10Parser.h"

Parser::Parser(const std::string &filename) : parser(get_parser(filename)) {}

Parser::~Parser() {
    delete parser;
}

IParser *Parser::get_parser(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file.");

    if (filename.find(".fast") != std::string::npos)
        return new FastParser(file);
    else if (filename.find(".graph") != std::string::npos)
        return new Dimacs10Parser(file, filename);
    else if (filename.find(".gra") != std::string::npos)
        return new DimacsParser(file, filename);
    else
        throw std::runtime_error("Unrecognized file format");
}

Graph Parser::parse() {
    auto t1 = std::chrono::high_resolution_clock::now();
    Graph g = parser->parse();
    auto t2 = std::chrono::high_resolution_clock::now();
    milliseconds = std::chrono::duration<double, std::milli>(t2 - t1).count();
    return g;
}
