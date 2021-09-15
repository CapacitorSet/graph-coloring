#ifndef GRAPH_COLORING_RANGESPLITTER_H
#define GRAPH_COLORING_RANGESPLITTER_H

#include "span-lite.hpp"
#include <cmath>
#include <vector>

class RangeSplitter {
    int max;
    int items_per_range;

public:
    RangeSplitter(int max, int num_ranges) : max(max) {
        items_per_range = std::ceil(float(max)/float(num_ranges));
    };

    // Get the lower end for the i-th range
    inline int get_min(int index) const {
        return std::min(items_per_range * index, max);
    };
    // Get the higher end for the i-th range
    inline int get_max(int index) const {
        return std::min(items_per_range * (index+1), max);
    };
    // Get whether the i-th range is empty
    inline bool is_empty(int index) const {
        return get_min(index) == get_max(index);
    };
};

template<typename T>
class VectorSplitter {
    RangeSplitter rs;
    std::vector<T> &vec;

public:
    VectorSplitter(std::vector<T> &vec, int num_ranges) : rs(vec.size(), num_ranges), vec(vec) {};

    // Get the i-th range
    nonstd::span<T> get(int index) {
        if (rs.is_empty(index))
            return nonstd::span<T>();
        return nonstd::span<T>(vec.begin() + rs.get_min(index), vec.begin() + rs.get_max(index));
    };
};


#endif //GRAPH_COLORING_RANGESPLITTER_H
