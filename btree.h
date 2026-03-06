/*
 * btree.h
 * Disk-backed B-Tree with integer keys.
 * All node I/O goes through the DiskManager layer.
 */

#ifndef BTREE_H
#define BTREE_H

#include "disk_manager.h"

/*
 * BTree — handle for one disk-backed B-Tree instance.
 */
typedef struct {
    DiskManager *dm;          /* underlying storage          */
    long         root_offset; /* file offset of root (-1 if empty) */
} BTree;

/*
 * Create or open a B-Tree stored in `filepath`.
 * If the file exists, the tree is loaded; otherwise a new tree is created.
 * Returns NULL on failure.
 */
BTree *btree_open(const char *filepath);

/*
 * Close the B-Tree, flushing all metadata to disk.
 */
void btree_close(BTree *bt);

/*
 * Insert `key` into the B-Tree.
 * Duplicates are silently ignored.
 * Returns 0 on success, -1 on error.
 */
int btree_insert(BTree *bt, int key);

/*
 * Search for `key`.
 * Returns 1 if found, 0 if not found.
 */
int btree_search(BTree *bt, int key);

/*
 * Delete `key` from the B-Tree.
 * Returns 0 on success, -1 if not found.
 */
int btree_delete(BTree *bt, int key);

#endif /* BTREE_H */
