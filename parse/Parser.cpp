#include "Parser.h"

Parser::Parser(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file.");

    if (filename.find(".fast") != std::string::npos)
        parser = FastParser(file);
    else if (filename.find(".graph") != std::string::npos)
        parser = Dimacs10Parser(file, filename);
    else if (filename.find(".gra") != std::string::npos)
        parser = DimacsParser(file, filename);
    else
        throw std::runtime_error("Unrecognized file format");
}

Graph Parser::parse() {
    if (std::holds_alternative<Dimacs10Parser>(parser))
        return std::get<Dimacs10Parser>(parser).parse();
    if (std::holds_alternative<DimacsParser>(parser))
        return std::get<DimacsParser>(parser).parse();
    if (std::holds_alternative<FastParser>(parser))
        return std::get<FastParser>(parser).parse();
    throw std::runtime_error("No valid parser registered!");
}
