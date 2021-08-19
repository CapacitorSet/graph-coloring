# Parallel graph coloring

This project investigates the implementation of several parallel algorithms for graph coloring and compares them to a well-known sequential algorithm in terms of time and memory.

## Overview

The project is organized into a series of parsers (`parse/`), a series of solvers (`solve/`), and benchmarking utilities (`benchmark/`). `graph/` contains the data structure and some utility functions for storing graphs as an adjacency list; finally, `utils/PCVector.h` contains a queue that supports several producers and consumers, which is a common parallelization primitive.

### Graph representation

*To do*

### Parsers

We developed three parsers: one each for the DIMACS and DIMACS-10 formats, plus `FastParser` to work on binary files and achieve significantly higher parsing speeds.

Profiling shows that parsing DIMACS graphs takes a long time to tokenize lines (i.e. split the string of nodes into a vector of node IDs) and parsing numbers into ints. For this reason, once a DIMACS graph is parsed it is saved to disk in a simple binary format that can be deserialized very efficiently.

#### Dimacs10Parser

The DIMACS-10 format is essentially an adjancency list: the i-th row contains the list of neighbor IDs. For this reason, the parsing operation can be parallelized very easily: each line can be parsed independently, especially as `std::vector` allows for concurrent writes in different positions.

In our implementation, the main threads acts as a line tokenizer and as a producer, while a variable number of worker threads act as consumers. The consumers simply tokenize lines into a vector of neighbor IDs and write the adjacency list into the graph.

#### DimacsParser

The DIMACS format is an adjacency list like DIMACS-10, but it presents a key challenge in that only neighbors with a higher ID are represented (the lower ones being implicit). In principle, this requires a sequential read as each adjacency list must be synchronized with the preceding ones.

#### FastParser

We developed a simple binary format for graphs with the goal of improving parsing speed for large graphs. It is based on length-prefixed vectors: the first 4 bytes contain the number of nodes `n`, and are followed by a list of `n` nodes. Each node is represented by 4 bytes for the number of neighbors `m`, and `m` 4-byte neighbor IDs.

We provide a simple example of the graph for a triangle, with the DIMACS-10 file on the left and its `FastParser` representation (in base 10) on the right:

```
3 3 # 3 nodes, 3 edges                | 3 # 3 nodes
2 3 # Node 1 is connected to 2 and 3  | 2 2 3 # Node 1 has 2 neighbors: 2, 3.
3 1 # Node 2 is connected to 3 and 1  | 2 1 3 # Node 2 has 2 neighbors: 1, 3.
1 2 # Node 3 is connected to 1 and 2  | 2 1 2 # Node 3 has 2 neighbors: 1, 2.
```

This format allows us to skip line tokenization, number tokenization, number parsing and list sorting: we only need to allocate vectors with a known size and copy the adjacency list directly from the file.

### Solvers

#### Sequential

*To do*

#### Luby

*To do*

#### Jones

*To do*

#### LDF

*To do*

#### Random priority

*To do*

### Benchmarking

The Benchmark class holds a vector of pointers to `Solver`, the base class for all solvers, and calls the pure virtual method `solve()` to run each of them. For each run, the following parameters are measured:

```cpp
struct result {
    bool success;             // Whether the graph was colored correctly
    uint32_t num_colors;      // Number of colors used
    double milliseconds;      // Time elapsed
    uint64_t peak_mem_usage;  // Peak memory usage in bytes
};
```

The memory usage is monitored by spawning a `MemoryMonitor` thread that will read the current usage every 100 us. At this time only Linux is supported, and the measurement occurs by multiplying the number of pages for "data+stack" in `/proc/self/statm` by the page size.

### PCVector

`PCVector` implements the common producer-consumer scheme. However, it has some differences with respect to the standard solution based on circular buffers:

 - PCVector implements a queue of arbitrary length, so it does not have an `empty` semaphore.
 - `stop()` is used to signal that there are no more items to be produced. This lets workers exit when the queue is empty.

## Results