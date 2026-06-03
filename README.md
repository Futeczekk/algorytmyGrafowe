# Graph Algorithms Performance Analyzer

This is a C++ university project focused on implementing and comparing selected graph algorithms.

The program works with two graph representations:
- adjacency list
- adjacency matrix

The main goal was to check how different algorithms behave depending on the graph representation, number of vertices and graph density.

## Implemented algorithms

### Minimum Spanning Tree
- Prim's algorithm
- Kruskal's algorithm

### Shortest Path
- Dijkstra's algorithm
- Bellman-Ford algorithm

## Features

- loading graphs from text files
- generating random graphs
- displaying graphs as an adjacency list and adjacency matrix
- running algorithms on both representations
- measuring execution time
- saving benchmark results to CSV files

## Output files

The program can generate benchmark results in CSV format:

```text
wyniki_mst.csv
wyniki_sciezki.csv
