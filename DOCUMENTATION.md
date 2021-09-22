# Parallel Graph Coloring

This project investigates the implementation of several parallel algorithms for graph coloring and compares them to a well-known sequential algorithm in terms of time and memory.

## Introduction to graph coloring
In discrete mathematics, a graph is a structure consists of nodes called “Vertices” and links between these nodes called “Edges”. There are two main types of a graph, which are undirected graph and directed graphs, where an edge between 2 vertices has a direction. However, in our scope we focus only on undirected graphs.

Graph components such as vertex or edge can have a label and the process of assigning label to an edge or vertex is called “Graph Labeling” and a label is traditionally represented by an integer. In our project, we focus on vertex labeling only and the labels we are assigning are colors, then the process is called “Vertices Coloring” or generally “Graph Coloring”.

Graph Coloring is a well-known problem in the discrete mathematics field and its practical applications. It is a way to color the vertices of a graph in such a way that there are no two adjacent vertices have the same color. Therefore, there are many algorithms have been developed to solve this problem. Our Goal is to find the most appropriate on for a parallel environment.

In order to solve the problem of graph coloring in an efficient way, the parallel coloring is used by most of algorithms, which manipulate the multi-core ability of modern processors and multi-thread programming capabilities provided by operating systems to achieve higher performance.
The key measures of the performance of an algorithm are:
-	Time consumption (Speed).
-	Memory consumption.
-	Colors consumption.

Therefore, an efficient algorithm with high performance should achieve low time consumption (high speed), low memory consumption, and low colors consumption, which means the algorithm should use the least number of colors to color the whole graph.

## Overview

The project is organized into parsers (`parse/`), solvers (`solve/`), parallelization utilities (`utils/`), and benchmarking utilities (`benchmark/`). Finally, `graph/` contains the data structure and some utility functions for storing graphs as an adjacency list.

### Parallelization primitives

#### PCVector

`PCVector` implements the common producer-consumer scheme. However, it has some differences with respect to the standard solution based on circular buffers:

 - PCVector implements a queue of arbitrary length, so it does not have an `empty` semaphore.
 - `stop()` is used to signal that there are no more items to be produced. This lets workers exit when the queue is empty.

We note that PCVector may be used to build an efficient thread pool mechanism. This primitive is not exposed directly as it is not used elsewhere, but it forms the basis for the `onReceive` method: it maintains a pool of consumers with a given callback.

#### RangeSplitter

`RangeSplitter` is used to split a numeric range into equal parts, accounting for possible edge cases. `VectorSplitter` does the same with vectors, and returns a span for a given thread to work on (we imported the span-lite library to enable support for `std::span`).

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

Profiling shows that very little time is actually spent in `remove_edges`, so this step was not parallelized. `probabilistic_select` on the other hand accounts for ~35% of the total runtime, and was parallelized with ease due to its unsynchronized nature: the list of candidates is split equally among threads.

Because this algorithm suffers from frequent looping, we added a small optimization: each thread in `probabilistic_select` must select at least one item. Doing so has resulted in an appreciable increase in performance.

#### Jones

`JonesSolver` implements the algorithm from *A parallel graph coloring heuristic*, M. Jones and T. Plassmann, 1992. At its core, Jones' algorithm defines a total ordering on the vertices `rho`, and then colors each node for which all uncolored neighbors have a lower `rho`.

Although Jones' paper describes the algorithm in terms of message sending between different processors - one per vertex - we adopt a cleaner implementation. Notably, `rho` is generated once by a single thread, and message-passing is replaced by keeping a queue of "free vertices" that can be colored. Parallelizing this is trivial: we can have as many threads as we want acting as consumers on the free vertices queue. Because multiple threads may update the `num_wait` for a vertex (the number of uncolored neighbors with a lower rho), it must use an atomic int; no further synchronization is required.

#### Largest Degree First (LDF)

`LDFsolver` implements the algorithm from *A Comparison of Parallel Graph Coloring Algorithms*, J. R. Allwright, 1995. As it is clear from it is name, the algorithm uses the degree of vertices in the subsets of the graph to decide which vertex to be colored before the other(Largest degree is colored firstly). This is a totlly different criterion from the independent set and random weights used in Jones and Lubys algorithms. In case of there are more than one vertex has the same degree, the priority of coloring is random in this case. The approach in LDF is to use the least number of colors. 

#### Smallest Degree Last (SDL)

`SDL` implements the algorithm from *A Comparison of Parallel Graph Coloring Algorithms*, J. R. Allwright, 1995.This algorithm uses degrees also like LDF. However, unlike LDF it uses also weights to decide which vertex to be colored first. The algorithm goes into two phases. The first phase is the weighting phase, in which, all vertices in the subset have a degree equal to the smallest degree takes a weight equal the current weight, and this vertex is removed from our calculation for the comming vertices which decrease the degree of its neighbor. This processed is repeated many times until we have groups of vertices each one have a unique weight. Then, the second phase comes to color this groups starting from the largest weight to the smallest weight.

### Other Two Solvers
These two algorithms never mentioned before in any scientific paper. However we have invented and implemented them for the comparing and analysis purposes.

#### First Vertex First (FVF)
 This algorithm adopts the order of the vertices data is saved like in the graph file. As its name, it colors the first vertex in the graph file in the subset of a thread. This approach makes the algorithm easier and simpler. This is because calculating degrees and weights and following a new order are operations that consume effort and time, so they are ignored in this algorithm to achieve the least possible time. However, this improvment in the performance may come with a trade off. There is no criterion theoritically planned to acheive the least number of colors. However, empirically, it may show different results.

#### Random Selection
 As its name, unlike LDF and SDL, it has no criterion to select a vertex to be color before other. The selection criterion is totally random. We shuffle the order of the vertices in the subset of a thread, then we start coloring in this random order. The approach is to save the effort consumed to follow a criterion. However, this improvment in the performance may come with a trade off. There is no criterion theoritically planned to acheive the least number of colors. However, empirically, it may show different results.



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

## Results

We analyzed the solvers with respect to time elapsed in a Jupyter notebook in `data-analysis/`. We report our conclusion, and redirect the reader to the notebook for further detail:

>The choice of algorithm results in vastly different performances. Early multithread algorithms found in literature may perform significantly worse than simpler single-thread ones, and thus be only of theoretical interest; more recent ones typically provide a measurable speedup.

>At the same time, one must not forget that efficient algorithms and techniques can provide a speedup simply at the expense of additional complexity or memory usage. The case of FastParser provides a strong argument for efficient algorithms; the case of JonesSolver shows that at times multithreading can beat even the sequential solver with the best heuristics.

>Ultimately, the question of whether to parallelize an algorithm or to implement a new one is hard to answer a priori, and the decision must be driven by profiling and experimentation.