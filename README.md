# Parallel graph coloring

This project implements a well-known sequential algorithm and several parallel ones for coloring graphs.

This is a short "user's manual"; for information on the algorithms and the design choices refer to DOCUMENTATION.md.

## Compile (Linux)

This is a standard CMake project; just build it out-of-tree and compile it with `make`.

```
mkdir build
cd build
cmake ..
make
cd ..
```

## Run

```
build/graph_coloring path_to_graph.gra
```
