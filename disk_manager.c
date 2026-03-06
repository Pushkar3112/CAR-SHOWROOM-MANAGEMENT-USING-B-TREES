/*
 * disk_manager.c
 * Implementation of the disk I/O layer for persistent B-Tree storage.
 *
 * File layout:
 *   [FileHeader] [Node_0] [Node_1] [Node_2] ...
 *
 * Each node occupies exactly sizeof(DiskNode) bytes.
 * Node at index i lives at offset: sizeof(FileHeader) + i * sizeof(DiskNode).
 */

#include "disk_manager.h"
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  Internal helper: compute file offset for the n-th node            */
/* ------------------------------------------------------------------ */
static long node_offset(int index)
{
    return (long)sizeof(FileHeader) + (long)index * (long)sizeof(DiskNode);
}

/* ------------------------------------------------------------------ */
/*  dm_open                                                           */
/* ------------------------------------------------------------------ */
DiskManager *dm_open(const char *filepath)
{
    DiskManager *dm = (DiskManager *)malloc(sizeof(DiskManager));
    if (!dm) {
        fprintf(stderr, "[disk_manager] Memory allocation failed.\n");
        return NULL;
    }
    strncpy(dm->filepath, filepath, sizeof(dm->filepath) - 1);
    dm->filepath[sizeof(dm->filepath) - 1] = '\0';

    dm->current_time = 0;
    for (int i = 0; i < CACHE_SIZE; i++) {
        dm->cache[i].offset = -1;
        dm->cache[i].is_dirty = 0;
        dm->cache[i].pin_count = 0;
        dm->cache[i].last_used = 0;
    }

    /* Try to open an existing file first (read + write, binary) */
    dm->fp = fopen(filepath, "r+b");

    if (dm->fp) {
        /* File exists — read and validate the header */
        if (dm_read_header(dm) != 0 || dm->header.magic != DM_MAGIC) {
            fprintf(stderr, "[disk_manager] Invalid or corrupt storage file.\n");
            fclose(dm->fp);
            free(dm);
            return NULL;
        }
    } else {
        /* File does not exist — create a new one */
        dm->fp = fopen(filepath, "w+b");
        if (!dm->fp) {
            fprintf(stderr, "[disk_manager] Cannot create file: %s\n", filepath);
            free(dm);
            return NULL;
        }

        /* Initialize a fresh header */
        dm->header.magic       = DM_MAGIC;
        dm->header.order       = BTREE_ORDER;
        dm->header.root_offset = -1;  /* empty tree */
        dm->header.node_count  = 0;

        if (dm_write_header(dm) != 0) {
            fprintf(stderr, "[disk_manager] Failed to write initial header.\n");
            fclose(dm->fp);
            free(dm);
            return NULL;
        }
    }

    return dm;
}

/* ------------------------------------------------------------------ */
/*  Cache eviction helper                                             */
/* ------------------------------------------------------------------ */
static void flush_cache_entry(DiskManager *dm, int idx)
{
    if (dm->cache[idx].offset != -1 && dm->cache[idx].is_dirty) {
        fseek(dm->fp, dm->cache[idx].offset, SEEK_SET);
        fwrite(&dm->cache[idx].node, sizeof(DiskNode), 1, dm->fp);
        dm->cache[idx].is_dirty = 0;
    }
}

/* ------------------------------------------------------------------ */
/*  dm_close                                                          */
/* ------------------------------------------------------------------ */
void dm_close(DiskManager *dm)
{
    if (!dm) return;

    /* Flush all dirty cache entries */
    for (int i = 0; i < CACHE_SIZE; i++) {
        flush_cache_entry(dm, i);
    }

    /* Always persist the latest header before closing */
    dm_write_header(dm);

    if (dm->fp) {
        fclose(dm->fp);
        dm->fp = NULL;
    }
    free(dm);
}

/* ------------------------------------------------------------------ */
/*  dm_read_header                                                    */
/* ------------------------------------------------------------------ */
int dm_read_header(DiskManager *dm)
{
    if (!dm || !dm->fp) return -1;

    if (fseek(dm->fp, 0L, SEEK_SET) != 0) return -1;

    size_t n = fread(&dm->header, sizeof(FileHeader), 1, dm->fp);
    return (n == 1) ? 0 : -1;
}

/* ------------------------------------------------------------------ */
/*  dm_write_header                                                   */
/* ------------------------------------------------------------------ */
int dm_write_header(DiskManager *dm)
{
    if (!dm || !dm->fp) return -1;

    /* We don't flush file header to disk aggressively anymore */
    /* It's done at shutdown or explicitly. */
    if (fseek(dm->fp, 0L, SEEK_SET) != 0) return -1;

    size_t n = fwrite(&dm->header, sizeof(FileHeader), 1, dm->fp);
    return (n == 1) ? 0 : -1;
}

/* ------------------------------------------------------------------ */
/*  Cache lookup helper                                               */
/* ------------------------------------------------------------------ */
static int get_cache_slot(DiskManager *dm, long offset)
{
    dm->current_time++;

    /* 1. Check if it's already in cache */
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (dm->cache[i].offset == offset) {
            dm->cache[i].last_used = dm->current_time;
            return i;
        }
    }

    /* 2. Find an empty slot or LRU */
    int lru_idx = -1;
    unsigned long min_time = -1;

    for (int i = 0; i < CACHE_SIZE; i++) {
        if (dm->cache[i].offset == -1) {
            return i;
        }
        if (dm->cache[i].last_used < min_time) {
            min_time = dm->cache[i].last_used;
            lru_idx = i;
        }
    }

    /* Evict LRU */
    flush_cache_entry(dm, lru_idx);
    dm->cache[lru_idx].offset = -1;
    return lru_idx;
}

/* ------------------------------------------------------------------ */
/*  dm_read_node                                                      */
/* ------------------------------------------------------------------ */
int dm_read_node(DiskManager *dm, long offset, DiskNode *node)
{
    if (!dm || !dm->fp || !node || offset < 0) return -1;

    int slot = get_cache_slot(dm, offset);
    
    if (dm->cache[slot].offset == offset) {
        /* Cache hit */
        *node = dm->cache[slot].node;
        return 0;
    }

    /* Cache miss -> load from disk */
    if (fseek(dm->fp, offset, SEEK_SET) != 0) return -1;
    size_t n = fread(node, sizeof(DiskNode), 1, dm->fp);
    if (n != 1) return -1;

    /* Store in cache */
    dm->cache[slot].offset = offset;
    dm->cache[slot].node = *node;
    dm->cache[slot].is_dirty = 0;
    dm->cache[slot].last_used = dm->current_time;

    return 0;
}

/* ------------------------------------------------------------------ */
/*  dm_write_node                                                     */
/* ------------------------------------------------------------------ */
int dm_write_node(DiskManager *dm, const DiskNode *node)
{
    if (!dm || !dm->fp || !node || node->self_offset < 0) return -1;

    int slot = get_cache_slot(dm, node->self_offset);

    dm->cache[slot].offset = node->self_offset;
    dm->cache[slot].node = *node;
    dm->cache[slot].is_dirty = 1;
    dm->cache[slot].last_used = dm->current_time;

    return 0;
}

/* ------------------------------------------------------------------ */
/*  dm_alloc_node                                                     */
/* ------------------------------------------------------------------ */
long dm_alloc_node(DiskManager *dm)
{
    if (!dm || !dm->fp) return -1;

    int index = dm->header.node_count;
    long offset = node_offset(index);

    /* Initialize an empty node */
    DiskNode blank;
    memset(&blank, 0, sizeof(DiskNode));
    blank.self_offset = offset;
    blank.is_leaf     = 1;
    blank.num_keys    = 0;
    for (int i = 0; i < BTREE_MAX_CHILD; i++) {
        blank.children[i] = -1;  /* no children */
    }

    dm->header.node_count++;

    int slot = get_cache_slot(dm, offset);
    dm->cache[slot].offset = offset;
    dm->cache[slot].node = blank;
    dm->cache[slot].is_dirty = 1;
    dm->cache[slot].last_used = dm->current_time;

    return offset;
}
