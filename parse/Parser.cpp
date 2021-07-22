#include "Parser.h"

Parser::Parser(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file.");

    if (filename.find(".fast") == std::string::npos)
        parser = MetisParser(std::move(file), filename);
    else
        parser = FastParser(std::move(file));
}

Graph Parser::parse() {
    if (std::holds_alternative<MetisParser>(parser))
        return std::get<MetisParser>(parser).parse();
    if (std::holds_alternative<FastParser>(parser))
        return std::get<FastParser>(parser).parse();
    throw std::runtime_error("No valid parser registered!");
}
