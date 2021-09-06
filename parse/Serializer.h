#ifndef GRAPH_COLORING_SERIALIZER_H
#define GRAPH_COLORING_SERIALIZER_H

#include <ostream>
#include <string>
#include "../graph/Graph.h"

// Serializes a given graph to a stream (typically a file) for usage with FastParser.
class Serializer {
private:
    const Graph &graph;
    std::ostream &ostream;

    // Serialize a single vertex (or generic u32)
    void serialize(uint32_t vertex);

    // Serialize as a length-prefixed vector
    template<typename T>
    void serialize(std::vector<T> vector) {
        // Length prefix (standardized to u32)
        serialize(static_cast<uint32_t>(vector.size()));
        // Then serialize each member (possibly recursively, eg. if we have a vector of adjacency lists)
        for (const T &item : vector)
            serialize(item);
    }

public:
    Serializer(const Graph &, std::ostream &);
};

#endif //GRAPH_COLORING_SERIALIZER_H
