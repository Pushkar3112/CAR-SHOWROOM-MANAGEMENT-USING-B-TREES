/*
 * main.c
 * Driver program for the refactored B-Tree project.
 *
 * Demonstrates:
 *   1. Persistent disk-backed B-Tree (insert, close, reopen, search)
 *   2. BST vs B-Tree performance benchmarks
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btree.h"
#include "bst.h"
#include "benchmark.h"

/* Default paths */
#define STORAGE_FILE   "data/btree_storage.bin"
#define DATASET_SIZE   100000

/* ------------------------------------------------------------------ */
/*  Demonstrate persistence: insert, close, reopen, verify             */
/* ------------------------------------------------------------------ */
static void demo_persistence(void)
{
    printf("==========================================================\n");
    printf("  Disk-Based B-Tree Persistence Demo\n");
    printf("==========================================================\n\n");

    /* Remove any previous file so the demo starts clean */
    remove(STORAGE_FILE);

    /* --- Phase 1: Insert keys and close --- */
    printf("[Phase 1] Creating B-Tree and inserting keys 1..20\n");

    BTree *bt = btree_open(STORAGE_FILE);
    if (!bt) {
        fprintf(stderr, "ERROR: Cannot open B-Tree storage.\n");
        return;
    }

    for (int i = 1; i <= 20; i++)
        btree_insert(bt, i);

    printf("  Inserted 20 keys. Closing tree (data written to disk).\n\n");
    btree_close(bt);

    /* --- Phase 2: Reopen and verify --- */
    printf("[Phase 2] Reopening tree from disk and verifying...\n");

    bt = btree_open(STORAGE_FILE);
    if (!bt) {
        fprintf(stderr, "ERROR: Cannot reopen B-Tree storage.\n");
        return;
    }

    int found = 0, missing = 0;
    for (int i = 1; i <= 20; i++) {
        if (btree_search(bt, i))
            found++;
        else
            missing++;
    }

    printf("  Keys found:   %d / 20\n", found);
    printf("  Keys missing: %d / 20\n", missing);

    if (missing == 0)
        printf("  >> Persistence verification PASSED!\n\n");
    else
        printf("  >> Persistence verification FAILED.\n\n");

    /* --- Phase 3: Delete some keys and verify --- */
    printf("[Phase 3] Deleting keys 5, 10, 15, 20...\n");

    btree_delete(bt, 5);
    btree_delete(bt, 10);
    btree_delete(bt, 15);
    btree_delete(bt, 20);

    printf("  Search key 5:  %s\n", btree_search(bt, 5)  ? "FOUND" : "NOT FOUND (correct)");
    printf("  Search key 10: %s\n", btree_search(bt, 10) ? "FOUND" : "NOT FOUND (correct)");
    printf("  Search key 7:  %s\n", btree_search(bt, 7)  ? "FOUND (correct)" : "NOT FOUND");
    printf("  Search key 14: %s\n", btree_search(bt, 14) ? "FOUND (correct)" : "NOT FOUND");

    printf("\n  Closing tree.\n\n");
    btree_close(bt);

    /* --- Phase 4: Reopen after delete and verify deletions persisted --- */
    printf("[Phase 4] Reopening after deletion to verify persistence...\n");

    bt = btree_open(STORAGE_FILE);
    if (!bt) {
        fprintf(stderr, "ERROR: Cannot reopen B-Tree storage.\n");
        return;
    }

    int del_ok = 1;
    int deleted_keys[] = {5, 10, 15, 20};
    for (int i = 0; i < 4; i++) {
        if (btree_search(bt, deleted_keys[i])) {
            printf("  ERROR: Deleted key %d still found!\n", deleted_keys[i]);
            del_ok = 0;
        }
    }

    int remaining_ok = 1;
    int remaining[] = {1, 2, 3, 4, 6, 7, 8, 9, 11, 12, 13, 14, 16, 17, 18, 19};
    for (int i = 0; i < 16; i++) {
        if (!btree_search(bt, remaining[i])) {
            printf("  ERROR: Expected key %d not found!\n", remaining[i]);
            remaining_ok = 0;
        }
    }

    if (del_ok && remaining_ok)
        printf("  >> Deletion persistence verification PASSED!\n");
    else
        printf("  >> Deletion persistence verification FAILED.\n");

    btree_close(bt);

    printf("\n==========================================================\n\n");
}

/* ------------------------------------------------------------------ */
/*  main                                                               */
/* ------------------------------------------------------------------ */
int main(void)
{
    printf("\n");
    printf("  Refactored B-Tree Project\n");
    printf("  Persistent Storage | Modular Design | Benchmarking\n");
    printf("\n");

    /* 1. Demonstrate persistence */
    demo_persistence();

    /* 2. Run BST vs B-Tree benchmarks */
    printf("Starting BST vs B-Tree benchmark (dataset: %d keys)...\n", DATASET_SIZE);
    run_benchmarks(DATASET_SIZE);

    printf("\nDone.\n");
    return 0;
}
