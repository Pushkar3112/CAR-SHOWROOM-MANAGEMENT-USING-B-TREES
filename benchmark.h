/*
 * benchmark.h
 * Performance benchmarking: BST vs disk-backed B-Tree.
 */

#ifndef BENCHMARK_H
#define BENCHMARK_H

/*
 * Run a full benchmark comparing BST and B-Tree
 * for insert, search, and delete on `dataset_size` keys.
 * Prints formatted results to stdout.
 */
void run_benchmarks(int dataset_size);

#endif /* BENCHMARK_H */
