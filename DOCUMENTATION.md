# Parallel graph coloring

This project investigates the implementation of several parallel algorithms for graph coloring and compares them to a well-known sequential algorithm in terms of time and memory.

## Overview

The project is organized into parsers (`parse/`), solvers (`solve/`), and benchmarking utilities (`benchmark/`). `graph/` contains the data structure and some utility functions for storing graphs as an adjacency list; finally, `utils/PCVector.h` contains a queue that supports several producers and consumers, which is a common parallelization primitive.

### Graph representation

Among the three common data structures for graph representation (adjacency list, adjacency matrix or incidence matrix), memory constraints require us to use the adjacency list format, given the requirement to process graphs with several millions of nodes and edges.

The "Compressed Sparse Row" (Yale) format was taken into consideration. Although it presents a small improvement in memory usage by allocating a contiguous array of edges, expressing the neighbor list in code requires either `std::span` (a C++20 feature which many users may not have access to) or impractical alternatives (constructing a `std::vector` on the fly, returning a pair of iterators, or developing a custom version of `std::span`), it was discarded.

We did however add a small improvement the data structure. MIS-based algorithms (eg. Luby) constantly delete vertices from the graph; this operation is rather expensive, requiring one to update the adjacency lists for the neighbors, and especially so in our case, where removing a vertex invalidates subsequent vertex IDs which must be then decremented. For this reason we do not actually delete vertices from the data structure, but rather add a `std::bitset deleted` on top of it which allows for fast deletion. It comes of course with the small downside that vertices must be checked against the bitset before they can be used, but profiling shows that this is not an issue at this time.

### Parsers

We developed three parsers: one each for the DIMACS and DIMACS-10 formats, plus `FastParser` to work on binary files and achieve significantly higher parsing speeds.

Profiling shows that parsing DIMACS graphs takes a long time to tokenize lines (i.e. split the string of nodes into a vector of node IDs) and parsing numbers into ints. For this reason, once a DIMACS graph is parsed it is saved to disk in a simple binary format (length-prefixed vectors) that can be deserialized very efficiently.

#### Dimacs10Parser

The DIMACS-10 format is essentially an adjancency list: the i-th row contains the list of neighbor IDs. For this reason, the parsing operation can be parallelized very easily: each line can be parsed independently, especially as `std::vector` allows for concurrent writes in different positions.

In our implementation, the main threads acts as a line tokenizer and as a producer, while a variable number of worker threads act as consumers. The consumers simply tokenize lines into a vector of neighbor IDs and write the adjacency list into the graph.

#### DimacsParser

The DIMACS format is an adjacency list like DIMACS-10, but it presents a key challenge in that only neighbors with a higher ID are represented (the lower ones being implicit). In principle, this would require a sequential read as each adjacency list must be synchronized with the preceding ones.

We parallelized the parsing by splitting it in two steps: first we parse the lines into partial adjacency lists, then we merge them. Specifically, the parallelization strategy is simple: the array is split in equal parts, one per thread.

#### FastParser

We developed a simple binary format for graphs with the goal of improving parsing speed for large graphs. It is based on length-prefixed vectors: the first 4 bytes contain the number of nodes `n`, and are followed by a list of `n` adjacency lists. Each adjacency list is represented by 4 bytes for the number of neighbors `m`, and `m` 4-byte vertex IDs.

We provide a simple example of the graph for a triangle, with the DIMACS-10 file on the left and its `FastParser` representation (in base 10) on the right:

```
3 3 # 3 nodes, 3 edges                | 3 # 3 nodes
2 3 # Node 1 is connected to 2 and 3  | 2 2 3 # Node 1 has 2 neighbors: 2, 3.
3 1 # Node 2 is connected to 3 and 1  | 2 1 3 # Node 2 has 2 neighbors: 1, 3.
1 2 # Node 3 is connected to 1 and 2  | 2 1 2 # Node 3 has 2 neighbors: 1, 2.
```

This format allows us to skip line tokenization, number tokenization, number parsing and list sorting: we only need to allocate vectors with a known size and copy the adjacency list directly from the file. This parser is so fast that no parallelization is needed; furthermore, we estimate that the bottleneck is vector allocation, thus we expect no performance improvement from multithreading.

### Solvers

#### Sequential

`SequentialSolver` implements the simple, well-known greedy strategy described in the specifications document, and mentioned in literature in *A parallel graph coloring heuristic*, M. Jones and P. Plassmann, 1992. It is single-threaded, i.e. sequential.

#### Luby

`LubySolver` implements the algorithm from *A simple parallel algorithm for the Maximal Independent Set problem*, M. Luby, 1985. It features two parallel steps, which we named `probabilistic_select` and `remove_edges`, and one sequential step, where the MIS is constructed. We then add a sequential step that colors the MIS, not mentioned in Luby's paper because it does not address graph coloring directly.

*To do: parallelization*

#### Jones

*To do*

#### LDF

*To do*

#### Random priority

*To do*

### Benchmarking

The Benchmark class is instantiated with a vector of solvers which implement the common interface `Solver`. For each run, the following parameters are measured:

```cpp
struct result {
    bool success;             // Whether the graph was colored correctly
    uint32_t num_colors;      // Number of colors used
    double milliseconds;      // Time elapsed
    uint64_t peak_mem_usage;  // Peak memory usage in bytes
};
```

The memory usage is monitored by spawning a `MemoryMonitor` thread that will read the current usage every 100 us. At this time only Linux is supported via `/proc/self/statm`.

### PCVector

`PCVector` implements the common producer-consumer scheme. However, it has some differences with respect to the standard solution based on circular buffers:

 - PCVector implements a queue of arbitrary length, so it does not have an `empty` semaphore.
 - `stop()` is used to signal that there are no more items to be produced. This lets workers exit when the queue is empty.

## Results