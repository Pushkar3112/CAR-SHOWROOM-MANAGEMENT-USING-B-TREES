/*
 * disk_manager.h
 * Disk I/O layer for persistent B-Tree storage.
 * Manages reading/writing fixed-size nodes to a binary file.
 */

#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <stdio.h>

/* B-Tree order and derived constants */
#define BTREE_ORDER     100
#define BTREE_MAX_KEYS  (BTREE_ORDER - 1)  
#define BTREE_MAX_CHILD BTREE_ORDER       

/* Magic number to validate file format */
#define DM_MAGIC 0x42545245  /* "BTRE" */

/* Node Cache parameters */
#define CACHE_SIZE 1024

/*
 * FileHeader — stored at offset 0 of the binary file.
 * Contains tree metadata needed to reload the tree on startup.
 */
typedef struct {
    unsigned int magic;       /* DM_MAGIC for validation            */
    int          order;       /* B-Tree order (redundant but useful) */
    long         root_offset; /* File offset of the root node (-1 if empty) */
    int          node_count;  /* Total number of nodes allocated     */
} FileHeader;

/*
 * DiskNode — fixed-size on-disk representation of a B-Tree node.
 * Child offsets are stored as file positions (long), not pointers.
 * A child offset of -1 means "no child".
 */
typedef struct {
    int  num_keys;                    /* Number of keys currently stored */
    int  is_leaf;                     /* 1 if leaf, 0 otherwise         */
    int  keys[BTREE_MAX_KEYS];       /* Key array                      */
    long children[BTREE_MAX_CHILD];   /* File offsets of child nodes    */
    long self_offset;                 /* This node's own file offset    */
} DiskNode;

/* Cache entry for LRU Buffer Pool */
typedef struct {
    long offset;
    DiskNode node;
    int is_dirty;
    int pin_count;
    unsigned long last_used;
} CacheEntry;

/*
 * DiskManager — handle for an open storage file.
 */
typedef struct {
    FILE       *fp;
    FileHeader  header;
    char        filepath[512];
    CacheEntry  cache[CACHE_SIZE];
    unsigned long current_time;
} DiskManager;

/* ---- API ---- */

/*
 * Open (or create) the storage file.
 * Returns a heap-allocated DiskManager, or NULL on failure.
 */
DiskManager *dm_open(const char *filepath);

/*
 * Close the storage file and free the DiskManager.
 * Writes the header before closing to persist metadata.
 */
void dm_close(DiskManager *dm);

/*
 * Read the node stored at `offset` into `node`.
 * Returns 0 on success, -1 on error.
 */
int dm_read_node(DiskManager *dm, long offset, DiskNode *node);

/*
 * Write `node` to disk at its self_offset position.
 * Returns 0 on success, -1 on error.
 */
int dm_write_node(DiskManager *dm, const DiskNode *node);

/*
 * Allocate a new node slot at the end of the file.
 * Initializes and writes a zeroed node. Returns the file offset,
 * or -1 on error.
 */
long dm_alloc_node(DiskManager *dm);

/*
 * Flush the file header to disk (offset 0).
 * Returns 0 on success, -1 on error.
 */
int dm_write_header(DiskManager *dm);

/*
 * Read the file header from disk into dm->header.
 * Returns 0 on success, -1 on error.
 */
int dm_read_header(DiskManager *dm);

#endif /* DISK_MANAGER_H */
