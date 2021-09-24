<h1 align="center">Parallel Graph Coloring</h1>

This project investigates the implementation of several parallel algorithms for graph coloring and compares them to a well-known sequential algorithm in terms of time and memory.

# Introduction to graph coloring
In discrete mathematics, a graph is a structure consists of nodes called “Vertices” and links between these nodes called “Edges”. There are two main types of a graph, which are undirected graph and directed graphs, where an edge between 2 vertices has a direction. However, in our scope we focus only on undirected graphs.

Graph components such as vertex or edge can have a label and the process of assigning label to an edge or vertex is called “Graph Labeling” and a label is traditionally represented by an integer. In our project, we focus on vertex labeling only and the labels we are assigning are colors, then the process is called “Vertices Coloring” or generally “Graph Coloring”.

Graph Coloring is a well-known problem in the discrete mathematics field and its practical applications. It is a way to color the vertices of a graph in such a way that there are no two adjacent vertices have the same color. Therefore, there are many algorithms have been developed to solve this problem. Our Goal is to find the most appropriate on for a parallel environment.

In order to solve the problem of graph coloring in an efficient way, the parallel coloring is used by most of algorithms, which manipulate the multi-core ability of modern processors and multi-thread programming capabilities provided by operating systems to achieve higher performance.

Therefore, in this project, we implement a group of well-known graph-coloring algorithms to analyse and compare them with each other, to know the performance of each one compared to the other.

# Overview

The project is organized into parsers (`parse/`), solvers (`solve/`), parallelization utilities (`utils/`), and benchmarking utilities (`benchmark/`). Finally, `graph/` contains the data structure and some utility functions for storing graphs as an adjacency list.

## Parallelization primitives

### PCVector

`PCVector` implements the common producer-consumer scheme. However, it has some differences with respect to the standard solution based on circular buffers:

 - PCVector implements a queue of arbitrary length, so it does not have an `empty` semaphore.
 - `stop()` is used to signal that there are no more items to be produced. This lets workers exit when the queue is empty.

We note that PCVector may be used to build an efficient thread pool mechanism. This primitive is not exposed directly as it is not used elsewhere, but it forms the basis for the `onReceive` method: it maintains a pool of consumers with a given callback.

### RangeSplitter

`RangeSplitter` is used to split a numeric range into equal parts, accounting for possible edge cases. `VectorSplitter` does the same with vectors, and returns a span for a given thread to work on (we imported the span-lite library to enable support for `std::span`).

## Graph representation

Among the three common data structures for graph representation (adjacency list, adjacency matrix or incidence matrix), memory constraints require us to use the adjacency list format, given the requirement to process graphs with several millions of nodes and edges.

The "Compressed Sparse Row" (Yale) format was taken into consideration. Although it presents a small improvement in memory usage by allocating a contiguous array of edges, expressing the neighbor list in code requires either `std::span` (a C++20 feature which many users may not have access to) or impractical alternatives (constructing a `std::vector` on the fly, returning a pair of iterators, or developing a custom version of `std::span`), it was discarded.

We did however add a small improvement the data structure. MIS-based algorithms (eg. Luby) constantly delete vertices from the graph; this operation is rather expensive, requiring one to update the adjacency lists for the neighbors, and especially so in our case, where removing a vertex invalidates subsequent vertex IDs which must be then decremented. For this reason we do not actually delete vertices from the data structure, but rather add a `std::bitset deleted` on top of it which allows for fast deletion. It comes of course with the small downside that vertices must be checked against the bitset before they can be used, but profiling shows that this is not an issue at this time.

## Parsers

We developed three parsers: one each for the DIMACS and DIMACS-10 formats, plus `FastParser` to work on binary files and achieve significantly higher parsing speeds.

Profiling shows that parsing DIMACS graphs takes a long time to tokenize lines (i.e. split the string of nodes into a vector of node IDs) and parsing numbers into ints. For this reason, once a DIMACS graph is parsed it is saved to disk in a simple binary format (length-prefixed vectors) that can be deserialized very efficiently.

### Dimacs10Parser

The DIMACS-10 format is essentially an adjancency list: the i-th row contains the list of neighbor IDs. For this reason, the parsing operation can be parallelized very easily: each line can be parsed independently, especially as `std::vector` allows for concurrent writes in different positions.

In our implementation, the main threads acts as a line tokenizer and as a producer, while a variable number of worker threads act as consumers. The consumers simply tokenize lines into a vector of neighbor IDs and write the adjacency list into the graph.

### DimacsParser

The DIMACS format is an adjacency list like DIMACS-10, but it presents a key challenge in that only neighbors with a higher ID are represented (the lower ones being implicit). In principle, this would require a sequential read as each adjacency list must be synchronized with the preceding ones.

We parallelized the parsing by splitting it in two steps: first we parse the lines into partial adjacency lists, then we merge them. Specifically, the parallelization strategy is simple: the array is split in equal parts, one per thread.

### FastParser

We developed a simple binary format for graphs with the goal of improving parsing speed for large graphs. It is based on length-prefixed vectors: the first 4 bytes contain the number of nodes `n`, and are followed by a list of `n` adjacency lists. Each adjacency list is represented by 4 bytes for the number of neighbors `m`, and `m` 4-byte vertex IDs.

We provide a simple example of the graph for a triangle, with the DIMACS-10 file on the left and its `FastParser` representation (in base 10) on the right:

```
3 3 # 3 nodes, 3 edges                | 3 # 3 nodes
2 3 # Node 1 is connected to 2 and 3  | 2 2 3 # Node 1 has 2 neighbors: 2, 3.
3 1 # Node 2 is connected to 3 and 1  | 2 1 3 # Node 2 has 2 neighbors: 1, 3.
1 2 # Node 3 is connected to 1 and 2  | 2 1 2 # Node 3 has 2 neighbors: 1, 2.
```

This format allows us to skip line tokenization, number tokenization, number parsing and list sorting: we only need to allocate vectors with a known size and copy the adjacency list directly from the file. This parser is so fast that no parallelization is needed; furthermore, we estimate that the bottleneck is vector allocation, thus we expect no performance improvement from multithreading.

## Solvers

### Sequential

`SequentialSolver` implements the simple, well-known greedy strategy described in the specifications document, and mentioned in literature in *A parallel graph coloring heuristic*, M. Jones and P. Plassmann, 1992. It is single-threaded, i.e. sequential.

### Luby

`LubySolver` implements the algorithm from *A simple parallel algorithm for the Maximal Independent Set problem*, M. Luby, 1985. It features two parallel steps, which we named `probabilistic_select` and `remove_edges`, and one sequential step, where the MIS is constructed. We then add a sequential step that colors the MIS, not mentioned in Luby's paper because it does not address graph coloring directly.

Profiling shows that very little time is actually spent in `remove_edges`, so this step was not parallelized. `probabilistic_select` on the other hand accounts for ~35% of the total runtime, and was parallelized with ease due to its unsynchronized nature: the list of candidates is split equally among threads.

Because this algorithm suffers from frequent looping, we added a small optimization: each thread in `probabilistic_select` must select at least one item. Doing so has resulted in an appreciable increase in performance.

### Jones

`JonesSolver` implements the algorithm from *A parallel graph coloring heuristic*, M. Jones and T. Plassmann, 1992. At its core, Jones' algorithm defines a total ordering on the vertices `rho`, and then colors each node for which all uncolored neighbors have a lower `rho`.

Although Jones' paper describes the algorithm in terms of message sending between different processors - one per vertex - we adopt a cleaner implementation. Notably, `rho` is generated once by a single thread, and message-passing is replaced by keeping a queue of "free vertices" that can be colored. Parallelizing this is trivial: we can have as many threads as we want acting as consumers on the free vertices queue. Because multiple threads may update the `num_wait` for a vertex (the number of uncolored neighbors with a lower rho), it must use an atomic int; no further synchronization is required.

### Largest Degree First (LDF)

`LDFsolver` implements the algorithm from *A Comparison of Parallel Graph Coloring Algorithms*, J. R. Allwright, 1995. As it is clear from it is name, the algorithm uses the degree of vertices in the subsets of the graph to decide which vertex to be colored before the other(Largest degree is colored firstly). This is a totlly different criterion from the independent set and random weights used in Jones and Lubys algorithms. In case of there are more than one vertex has the same degree, the priority of coloring is random in this case. The approach in LDF is to use the least number of colors. 

### Smallest Degree Last (SDL)

`SDL` implements the algorithm from *A Comparison of Parallel Graph Coloring Algorithms*, J. R. Allwright, 1995.This algorithm uses degrees also like LDF. However, unlike LDF it uses also weights to decide which vertex to be colored first. The algorithm goes into two phases. The first phase is the weighting phase, in which, all vertices in the subset have a degree equal to the smallest degree takes a weight equal the current weight, and this vertex is removed from our calculation for the comming vertices which decrease the degree of its neighbor. This processed is repeated many times until we have groups of vertices each one have a unique weight. Then, the second phase comes to color this groups starting from the largest weight to the smallest weight.

## Other Two Solvers
These two algorithms never mentioned before in any scientific paper. However we have invented and implemented them for the comparing and analysis purposes.

### First Vertex First (FVF)
 This algorithm adopts the order of the vertices data is saved like in the graph file. As its name, it colors the first vertex in the graph file in the subset of a thread. This approach makes the algorithm easier and simpler. This is because calculating degrees and weights and following a new order are operations that consume effort and time, so they are ignored in this algorithm to achieve the least possible time. However, this improvment in the performance may come with a trade off. There is no criterion theoritically planned to acheive the least number of colors. However, empirically, it may show different results.

### Random Selection
 As its name, unlike LDF and SDL, it has no criterion to select a vertex to be color before other. The selection criterion is totally random. We shuffle the order of the vertices in the subset of a thread, then we start coloring in this random order. The approach is to save the effort consumed to follow a criterion. However, this improvment in the performance may come with a trade off. There is no criterion theoritically planned to acheive the least number of colors. However, empirically, it may show different results.



## Benchmarking

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

# Results

The sequential algorithm and the six parallel algorithms have been implemented and their performance have been tested. The key measures - we use - of the performance of an algorithm are:
-	Time consumption (Speed).
-	Memory consumption.
-	Colors consumption.

Thus, an efficient algorithm with high performance should achieve low time consumption (high speed), low memory consumption, and low colors consumption, which means the algorithm should use the least number of colors to color the whole graph.

For the results, we would like firstly to mention the features of platform we used to get these results. It is Intel(R) Core(TM) i7-10750H CPU @ 2.60GHz 2.59 GHz processor and 16 GB RAM. Now, the results will be shown below in 3 tables, and each table will be followed with an analysis.


## Time Consumption


>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| Sequential ---------->| 78 ms | 188 ms | 364 ms | 753 ms | 1554 ms | 3367 ms | 7685 ms | 18075 ms |

>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| Luby (1) | 345 ms | 706 ms | 1503 ms | 3117 ms | 6894 ms | 15428 ms | 34067 ms | 79219 ms |
>| Jones-Plassmann (1) | 113 ms | 203 ms | 419 ms | 870 ms | 1774 ms | 3722 ms | 8012 ms | 16380 ms |
>| SDL (1) | 98 ms | 207 ms | 418 ms | 874 ms | 1813 ms | 3879 ms | 8129 ms | 17026 ms |
>| LDF (1) | 100 ms | 209 ms | 425 ms | 885 ms | 1877 ms | 3974 ms | 8478 ms | 17841 ms |
>| Random-Selection (1) | 87 ms | 177 ms | 374 ms | 772 ms | 1633 ms | 3501 ms | 7978 ms | 17958 ms |
>| FVF (1) | 75 ms | 172 ms | 341 ms | 702 ms | 1479 ms | 3136 ms | 6534 ms | 14051 ms |

>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| Luby (2) | 335 ms | 685 ms | 1425 ms | 3036 ms | 6690 ms | 14971 ms | 32858 ms | 75695 ms |
>| Jones-Plassmann (2) | 63 ms | 127 ms | 262 ms | 506 ms | 1052 ms | 2201 ms | 4514 ms | 9654 ms |
>| SDL (2) | 53 ms | 106 ms | 242 ms | 453 ms | 986 ms | 1996 ms | 4189 ms | 8808 ms |
>| LDF (2) | 55 ms | 123 ms | 251 ms | 458 ms | 955 ms | 2043 ms | 4379 ms | 9260 ms |
>| Random-Selection (2) | 48 ms | 90 ms | 215 ms | 409 ms | 860 ms | 1779 ms | 4009 ms | 9103 ms |
>| FVF (2) | 43 ms | 90 ms | 206 ms | 384 ms | 791 ms | 1617 ms | 3512 ms | 7178 ms |

>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| Luby (4) | 361 ms | 671 ms | 1401 ms | 3046 ms| 6653 ms | 14637 ms | 32858 ms | 71310 ms |
>| Jones-Plassmann (4) | 44 ms | 88 ms | 198 ms | 373 ms | 711 ms | 1476 ms | 3038 ms | 6338 ms |
>| SDL (4) | 38 ms | 60 ms | 153 ms | 278 ms | 547 ms | 1104 ms | 2233 ms | 4717 ms |
>| LDF (4) | 30 ms | 57 ms | 151 ms | 299 ms | 559 ms | 1120 ms | 2295 ms | 4948 ms |
>| Random-Selection (4) | 30 ms | 58 ms | 141 ms | 269 ms | 485 ms | 978 ms | 2142 ms | 4694 ms |
>| FVF (4) | 23 ms | 55 ms | 135 ms | 232 ms | 444 ms | 913 ms | 1832 ms | 3877 ms |

>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| Luby (6) | 356 ms | 678 ms | 1423 ms | 3073 ms | 6740 ms | 14878 ms | 32679 ms | 71684 ms |
>| Jones-Plassmann (6) | 38 ms | 88 ms | 174 ms | 328 ms | 619 ms | 1258 ms | 2536 ms | 5316 ms |
>| SDL (6) | 30 ms | 56 ms | 112 ms | 260 ms | 496 ms | 852 ms | 1664 ms | 3324 ms |
>| LDF (6) | 32 ms | 63 ms | 111 ms | 254 ms | 460 ms | 849 ms | 1723 ms | 3465 ms |
>| Random-Selection (6) | 24 ms | 47 ms | 116 ms | 220 ms | 462 ms | 772 ms | 1563 ms | 3348 ms |
>| FVF (6) | 26 ms | 40 ms | 84 ms | 220 ms | 409 ms | 765 ms | 1385 ms | 2824 ms |

In analyzing the above table, It is clear that FVF has the best time consumption over all other algorithms. However, the Luby algorithm has the worest time consumption it consumes very large amount of time. Even with 6 threads it consumes time more than the sequential algorithm.

For the other 4 parallel algorithm, they also acheive a very good time consumption compared to the sequential algorithm.

For SDL and LDF, they have similar results. However, SDL has a better time consumption specially with a big amount of data (large graphs). For the Random-Selection algorithm, it can be said that it has also a similar results compared to the SDL and FDL. For the Jones-Plassmann, it is the least one in time consumption after Luby. However, it still has a good performance compared to the sequential algorithms.

________

## Memory Consumption

>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| Sequential ----------> | 0 MB | 0 MB | 1 MB | 1 MB | 2 MB | 4 MB | 8 MB | 16 Mb |

>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| Luby (1) | 3 MB | 6 MB | 8 MB | 16 MB | 32 MB | 62 MB | 127 MB | 261 MB |
>| Jones-Plassmann (1) | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB |
>| SDL (1) | 0 MB | 0 MB | 1 MB | 2 MB | 4 MB | 8 MB | 16 MB | 32 MB |
>| LDF (1) | 0 MB | 0 MB | 1 MB | 2 MB | 3 MB | 7 MB | 13 MB | 24 MB |
>| Random-Selection (1) | 0 MB | 0 MB | 0 MB | 1 MB | 0 MB | 0 MB | 0 MB | 0 MB |
>| FVF (1) | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB |

>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| Luby (2) | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 1 MB | 46 MB |
>| Jones-Plassmann (2) | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB |
>| SDL (2) | 0 MB | 0 MB | 0 MB | 1 MB | 2 MB | 4 MB | 7 MB | 15 MB |
>| LDF (2) | 0 MB | 0 MB | 0 MB | 0 MB | 1 MB | 3 MB | 4 MB | 0 MB |
>| Random-Selection (2) | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB |
>| FVF (2) | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB | 0 MB |

>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| Luby (4) | 8 MB | 8 MB | 8 MB | 8 MB | 8 MB | 8 MB | 9 MB | 54 MB |
>| Jones-Plassmann (4) | 8 MB | 8 MB | 8 MB | 8 MB | 8 MB | 8 MB | 8 MB | 8 MB |
>| SDL (4) | 8 MB | 8 MB | 8 MB | 9 MB | 10 MB | 12 MB | 15 MB | 23 MB |
>| LDF (4) | 8 MB | 8 MB | 8 MB | 8 MB | 8 MB | 8 MB | 9 MB | 11 MB |
>| Random-Selection (4) | 8 MB | 8 MB | 8 MB | 8 MB | 8 MB | 8 MB | 8 MB | 8 MB |
>| FVF (4) | 8 MB | 8 MB | 8 MB | 8 MB | 8 MB | 8 MB | 8 MB | 8 MB |

>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| Luby (6) | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB | 70 MB |
>| Jones-Plassmann (6) | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB |
>| SDL (6) | 24 MB | 24 MB | 24 MB | 24 MB | 25 MB | 26 MB | 29 MB | 32 MB |
>| LDF (6) | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB |
>| Random-Selection (6) | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB |
>| FVF (6) | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB | 24 MB |

In memory consumption, Jones-Plassmann, FVF and Random-Selection are the least in memory consumpyion.

Luby and SDL are the least efficient ones in memory consumption.

However, LDF is moderate.
____________

## Colors Consumption

>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| Sequential ----------> | 14 | 18 | 16 | 19 | 20 | 20 | 21 | 22 |

>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| Luby (1) ------------> | 14 | 17 | 17 | 18 | 19 | 20 | 21 | 21 |
>| Luby (2) | 15 | 16 | 17 | 18 | 19 | 21 | 21 | 21 |
>| Luby (4) | 14 | 16 | 16 | 18 | 19 | 21 | 21 | 22 |
>| Luby (6) | 15 | 16 | 17 | 19 | 19 | 21 | 20 | 22 |

>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| Jones-Plassmann (1) | 15 | 17 | 17 | 18 | 19 | 21 | 20 | 21 |
>| Jones-Plassmann (2) | 15 | 17 | 17 | 18 | 19 | 21 | 20 | 21 |
>| Jones-Plassmann (4) | 15 | 17 | 17 | 18 | 19 | 21 | 20 | 21 |
>| Jones-Plassmann (6) | 15 | 17 | 17 | 18 | 19 | 21 | 20 | 21 |

>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| SDL (1) -------------> | 15 | 16 | 17 | 19 | 20 | 20 | 22 | 22 |
>| SDL (2) | 15 | 16 | 17 | 19 | 20 | 20 | 22 | 22 |
>| SDL (4) | 15 | 16 | 17 | 19 | 20 | 20 | 22 | 22 |
>| SDL (6) | 15 | 16 | 17 | 19 | 20 | 20 | 22 | 22 |

>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| LDF (1) -------------> | 14 | 15 | 16 | 17 | 19 | 19 | 20 | 21 |
>| LDF (2) | 14 | 15 | 16 | 17 | 19 | 19 | 20 | 21 |
>| LDF (4) | 14 | 15 | 16 | 17 | 19 | 19 | 20 | 21 |
>| LDF (6) | 14 | 15 | 16 | 17 | 19 | 19 | 20 | 21 |

>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| Random-Selection (1) | 14 | 18 | 16 | 19 | 20 | 20 | 21 | 22 |
>| Random-Selection (2) | 16 | 17 | 17 | 19 | 19 | 21 | 21 | 22 |
>| Random-Selection (4) | 14 | 16 | 17 | 18 | 20 | 22 | 20 | 22 |
>| Random-Selection (6) | 15 | 17 | 17 | 18 | 19 | 19 | 21 | 21 |

>| Algorithm | rgg_n 2^15 | rgg_n 2^16 | rgg_n 2^17 | rgg_n 2^18 | rgg_n 2^19 | rgg_n 2^20 | rgg_n 2^21 | rgg_n 2^22 |
>| --------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
>| FVF (1) -------------> | 14 | 16 | 17 | 18 | 20 | 19 | 22 | 22 |
>| FVF (2) | 14 | 16 | 17 | 18 | 20 | 19 | 22 | 22 |
>| FVF (4) | 14 | 16 | 17 | 18 | 20 | 19 | 22 | 22 |
>| FVF (6) | 14 | 16 | 17 | 18 | 20 | 19 | 22 | 22 |

In coloring consumption, The LDF is the best one, since it used the least number of colors in different-sized graphs and in all number of threads.

However, all other algorithms have similar consumption.