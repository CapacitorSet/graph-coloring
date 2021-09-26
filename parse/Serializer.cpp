#include "Serializer.h"

Serializer::Serializer(const Graph &graph, std::ostream &ostream) : graph(graph), ostream(ostream) {
    serialize(graph.neighbor_indices);
    // Ensure that the fastparse graph was written, so that crashes do not result in a malformed file
    ostream.flush();
}

void Serializer::serialize(uint32_t vertex) {
    ostream.write(reinterpret_cast<const char *>(&vertex), sizeof(uint32_t));
}
