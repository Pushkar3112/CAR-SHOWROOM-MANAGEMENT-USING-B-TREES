/*
 * btree.c
 * Disk-backed B-Tree implementation with integer keys.
 *
 * Every node read/write goes through the DiskManager.
 * Only modified nodes are written back to disk.
 */

#include "btree.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ================================================================== */
/*  Internal helpers                                                   */
/* ================================================================== */

/* Find position where `key` should be in the key array */
static int find_pos(const DiskNode *node, int key)
{
    int i = 0;
    while (i < node->num_keys && key > node->keys[i])
        i++;
    return i;
}

/* ================================================================== */
/*  Split a full child of `parent` at child index `idx`                */
/*  The child must have BTREE_MAX_KEYS keys (full).                    */
/* ================================================================== */
static int split_child(BTree *bt, DiskNode *parent, int idx)
{
    DiskNode child;
    if (dm_read_node(bt->dm, parent->children[idx], &child) != 0)
        return -1;

    int mid = BTREE_MAX_KEYS / 2;  /* index 2 for order-5 */

    /* Allocate a new sibling node */
    long sib_offset = dm_alloc_node(bt->dm);
    if (sib_offset < 0) return -1;

    DiskNode sibling;
    if (dm_read_node(bt->dm, sib_offset, &sibling) != 0) return -1;

    sibling.is_leaf  = child.is_leaf;
    sibling.num_keys = 0;

    /* Copy keys right of `mid` into sibling */
    for (int i = mid + 1; i < child.num_keys; i++) {
        sibling.keys[sibling.num_keys] = child.keys[i];
        sibling.values[sibling.num_keys] = child.values[i];
        sibling.num_keys++;
    }

    /* Copy child pointers if internal node */
    if (!child.is_leaf) {
        for (int i = mid + 1; i <= child.num_keys; i++) {
            sibling.children[i - mid - 1] = child.children[i];
        }
    }

    int promoted_key = child.keys[mid];
    long promoted_value = child.values[mid];
    child.num_keys = mid;  /* shrink original child */

    /* Shift parent's children and keys to make room */
    for (int i = parent->num_keys; i > idx; i--) {
        parent->keys[i]        = parent->keys[i - 1];
        parent->values[i]      = parent->values[i - 1];
        parent->children[i + 1] = parent->children[i];
    }

    parent->keys[idx]        = promoted_key;
    parent->values[idx]      = promoted_value;
    parent->children[idx + 1] = sib_offset;
    parent->num_keys++;

    /* Write all three modified nodes back to disk */
    if (dm_write_node(bt->dm, &child)    != 0) return -1;
    if (dm_write_node(bt->dm, &sibling)  != 0) return -1;
    if (dm_write_node(bt->dm, parent)    != 0) return -1;

    return 0;
}

/* ================================================================== */
/*  Insert key into a non-full node (recursive)                        */
/* ================================================================== */
static int insert_nonfull(BTree *bt, long node_offset, int key, long value)
{
    DiskNode node;
    if (dm_read_node(bt->dm, node_offset, &node) != 0)
        return -1;

    if (node.is_leaf) {
        /* Insert key in sorted position */
        int i = node.num_keys - 1;
        while (i >= 0 && key < node.keys[i]) {
            node.keys[i + 1] = node.keys[i];
            node.values[i + 1] = node.values[i];
            i--;
        }

        /* Duplicate check - update value */
        if (i >= 0 && node.keys[i] == key) {
            node.values[i] = value;
            return dm_write_node(bt->dm, &node);
        }

        node.keys[i + 1] = key;
        node.values[i + 1] = value;
        node.num_keys++;

        return dm_write_node(bt->dm, &node);
    }

    /* Internal node — find the child to descend into */
    int i = node.num_keys - 1;
    while (i >= 0 && key < node.keys[i])
        i--;

    /* Duplicate check in internal node - update value */
    if (i >= 0 && node.keys[i] == key) {
        node.values[i] = value;
        return dm_write_node(bt->dm, &node);
    }

    i++;

    /* Read the child to check if it's full */
    DiskNode child;
    if (dm_read_node(bt->dm, node.children[i], &child) != 0)
        return -1;

    if (child.num_keys == BTREE_MAX_KEYS) {
        /* Split the full child */
        if (split_child(bt, &node, i) != 0)
            return -1;

        /* Re-read node after split (parent was modified) */
        if (dm_read_node(bt->dm, node_offset, &node) != 0)
            return -1;

        /* Decide which child to descend into after the split */
        if (key > node.keys[i])
            i++;
        else if (key == node.keys[i]) {
            /* The newly promoted key IS our key; just update the value */
            node.values[i] = value;
            return dm_write_node(bt->dm, &node);
        }
    }

    return insert_nonfull(bt, node.children[i], key, value);
}

/* ================================================================== */
/*  Public API                                                         */
/* ================================================================== */

BTree *btree_open(const char *filepath)
{
    BTree *bt = (BTree *)malloc(sizeof(BTree));
    if (!bt) return NULL;

    bt->dm = dm_open(filepath);
    if (!bt->dm) {
        free(bt);
        return NULL;
    }

    bt->root_offset = bt->dm->header.root_offset;
    return bt;
}

void btree_close(BTree *bt)
{
    if (!bt) return;

    /* Persist root offset in the header before closing */
    bt->dm->header.root_offset = bt->root_offset;
    dm_close(bt->dm);
    free(bt);
}

int btree_insert(BTree *bt, int key, long value)
{
    if (!bt) return -1;

    /* Empty tree — create the root */
    if (bt->root_offset < 0) {
        long offset = dm_alloc_node(bt->dm);
        if (offset < 0) return -1;

        DiskNode root;
        if (dm_read_node(bt->dm, offset, &root) != 0) return -1;

        root.is_leaf    = 1;
        root.num_keys   = 1;
        root.keys[0]    = key;
        root.values[0]  = value;

        if (dm_write_node(bt->dm, &root) != 0) return -1;

        bt->root_offset = offset;
        bt->dm->header.root_offset = offset;
        dm_write_header(bt->dm);
        return 0;
    }

    /* Check if root is full */
    DiskNode root;
    if (dm_read_node(bt->dm, bt->root_offset, &root) != 0)
        return -1;

    if (root.num_keys == BTREE_MAX_KEYS) {
        /* Allocate a new root */
        long new_root_offset = dm_alloc_node(bt->dm);
        if (new_root_offset < 0) return -1;

        DiskNode new_root;
        if (dm_read_node(bt->dm, new_root_offset, &new_root) != 0) return -1;

        new_root.is_leaf     = 0;
        new_root.num_keys    = 0;
        new_root.children[0] = bt->root_offset;

        if (dm_write_node(bt->dm, &new_root) != 0) return -1;

        /* Split the old root */
        if (split_child(bt, &new_root, 0) != 0) return -1;

        bt->root_offset = new_root_offset;
        bt->dm->header.root_offset = new_root_offset;
        dm_write_header(bt->dm);

        return insert_nonfull(bt, new_root_offset, key, value);
    }

    return insert_nonfull(bt, bt->root_offset, key, value);
}

long btree_search(BTree *bt, int key)
{
    if (!bt || bt->root_offset < 0) return -1;

    long current = bt->root_offset;

    while (current >= 0) {
        DiskNode node;
        if (dm_read_node(bt->dm, current, &node) != 0)
            return 0;

        int i = find_pos(&node, key);

        if (i < node.num_keys && node.keys[i] == key)
            return node.values[i];  /* found */

        if (node.is_leaf)
            return -1;  /* not found */

        current = node.children[i];
    }

    return -1;
}

/* ================================================================== */
/*  Deletion helpers                                                   */
/* ================================================================== */

/* Helper structure to return both key and value */
typedef struct {
    int key;
    long value;
} KeyValue;

/* Find the predecessor key: rightmost key in the left subtree */
static KeyValue get_predecessor(BTree *bt, long offset)
{
    DiskNode node;
    dm_read_node(bt->dm, offset, &node);

    while (!node.is_leaf) {
        dm_read_node(bt->dm, node.children[node.num_keys], &node);
    }
    KeyValue kv = {node.keys[node.num_keys - 1], node.values[node.num_keys - 1]};
    return kv;
}

/* Find the successor key: leftmost key in the right subtree */
static KeyValue get_successor(BTree *bt, long offset)
{
    DiskNode node;
    dm_read_node(bt->dm, offset, &node);

    while (!node.is_leaf) {
        dm_read_node(bt->dm, node.children[0], &node);
    }
    KeyValue kv = {node.keys[0], node.values[0]};
    return kv;
}

/* Forward declaration */
static int delete_key(BTree *bt, long node_offset, int key);

/*
 * Ensure that the child at index `idx` has at least MIN_KEYS keys
 * (ceil(order/2) - 1 = 2 for order 5).
 * Borrows from a sibling or merges if necessary.
 */
static void merge_children(BTree *bt, DiskNode *parent, int idx)
{
    DiskNode child, right_sib;
    dm_read_node(bt->dm, parent->children[idx], &child);
    dm_read_node(bt->dm, parent->children[idx + 1], &right_sib);

    int orig_child_keys = child.num_keys;

    child.keys[child.num_keys] = parent->keys[idx];
    child.values[child.num_keys] = parent->values[idx];
    child.num_keys++;

    for (int i = 0; i < right_sib.num_keys; i++) {
        child.keys[child.num_keys] = right_sib.keys[i];
        child.values[child.num_keys] = right_sib.values[i];
        child.num_keys++;
    }

    if (!child.is_leaf) {
        for (int i = 0; i <= right_sib.num_keys; i++)
            child.children[orig_child_keys + 1 + i] = right_sib.children[i];
    }

    for (int i = idx; i < parent->num_keys - 1; i++) {
        parent->keys[i] = parent->keys[i + 1];
        parent->values[i] = parent->values[i + 1];
        parent->children[i + 1] = parent->children[i + 2];
    }
    parent->num_keys--;

    dm_write_node(bt->dm, &child);
    dm_write_node(bt->dm, parent);
}

static void fill_child(BTree *bt, DiskNode *parent, int idx)
{
    int min_keys = (BTREE_ORDER + 1) / 2 - 1;  /* 2 for order 5 */

    DiskNode left_sib, right_sib, child;
    dm_read_node(bt->dm, parent->children[idx], &child);

    /* Try borrowing from left sibling */
    if (idx > 0) {
        dm_read_node(bt->dm, parent->children[idx - 1], &left_sib);
        if (left_sib.num_keys > min_keys) {
            for (int i = child.num_keys - 1; i >= 0; i--) {
                child.keys[i + 1] = child.keys[i];
                child.values[i + 1] = child.values[i];
            }
            if (!child.is_leaf) {
                for (int i = child.num_keys; i >= 0; i--)
                    child.children[i + 1] = child.children[i];
                child.children[0] = left_sib.children[left_sib.num_keys];
            }

            child.keys[0] = parent->keys[idx - 1];
            child.values[0] = parent->values[idx - 1];
            parent->keys[idx - 1] = left_sib.keys[left_sib.num_keys - 1];
            parent->values[idx - 1] = left_sib.values[left_sib.num_keys - 1];

            child.num_keys++;
            left_sib.num_keys--;

            dm_write_node(bt->dm, &child);
            dm_write_node(bt->dm, &left_sib);
            dm_write_node(bt->dm, parent);
            return;
        }
    }

    /* Try borrowing from right sibling */
    if (idx < parent->num_keys) {
        dm_read_node(bt->dm, parent->children[idx + 1], &right_sib);
        if (right_sib.num_keys > min_keys) {
            child.keys[child.num_keys] = parent->keys[idx];
            child.values[child.num_keys] = parent->values[idx];
            if (!child.is_leaf)
                child.children[child.num_keys + 1] = right_sib.children[0];

            parent->keys[idx] = right_sib.keys[0];
            parent->values[idx] = right_sib.values[0];

            for (int i = 0; i < right_sib.num_keys - 1; i++) {
                right_sib.keys[i] = right_sib.keys[i + 1];
                right_sib.values[i] = right_sib.values[i + 1];
            }
            if (!right_sib.is_leaf) {
                for (int i = 0; i < right_sib.num_keys; i++)
                    right_sib.children[i] = right_sib.children[i + 1];
            }

            child.num_keys++;
            right_sib.num_keys--;

            dm_write_node(bt->dm, &child);
            dm_write_node(bt->dm, &right_sib);
            dm_write_node(bt->dm, parent);
            return;
        }
    }

    /* Merge: prefer merging with left sibling */
    if (idx > 0)
        merge_children(bt, parent, idx - 1);
    else
        merge_children(bt, parent, idx);
}

/* Recursive delete */
static int delete_key(BTree *bt, long offset, int key)
{
    DiskNode node;
    if (dm_read_node(bt->dm, offset, &node) != 0)
        return -1;

    int idx = find_pos(&node, key);
    int min_keys = (BTREE_ORDER + 1) / 2 - 1;

    if (idx < node.num_keys && node.keys[idx] == key) {
        /* Key found in this node */
        if (node.is_leaf) {
            for (int i = idx; i < node.num_keys - 1; i++) {
                node.keys[i] = node.keys[i + 1];
                node.values[i] = node.values[i + 1];
            }
            node.num_keys--;
            dm_write_node(bt->dm, &node);
            return 0;
        }

        /* Case 2: key is in an internal node */
        DiskNode left_child;
        dm_read_node(bt->dm, node.children[idx], &left_child);
        if (left_child.num_keys > min_keys) {
            KeyValue pred = get_predecessor(bt, node.children[idx]);
            node.keys[idx] = pred.key;
            node.values[idx] = pred.value;
            dm_write_node(bt->dm, &node);
            return delete_key(bt, node.children[idx], pred.key);
        }

        DiskNode right_child;
        dm_read_node(bt->dm, node.children[idx + 1], &right_child);
        if (right_child.num_keys > min_keys) {
            KeyValue succ = get_successor(bt, node.children[idx + 1]);
            node.keys[idx] = succ.key;
            node.values[idx] = succ.value;
            dm_write_node(bt->dm, &node);
            return delete_key(bt, node.children[idx + 1], succ.key);
        }

        /* Both children have min keys — merge them */
        merge_children(bt, &node, idx);
        return delete_key(bt, node.children[idx], key);
    }

    /* Key not found in this node */
    if (node.is_leaf)
        return -1;  /* key doesn't exist */

    /* Ensure child has enough keys before descending */
    DiskNode child;
    dm_read_node(bt->dm, node.children[idx], &child);

    if (child.num_keys <= min_keys) {
        fill_child(bt, &node, idx);
        /* Re-read parent after fill */
        dm_read_node(bt->dm, offset, &node);
        idx = find_pos(&node, key);
    }

    return delete_key(bt, node.children[idx], key);
}

int btree_delete(BTree *bt, int key)
{
    if (!bt || bt->root_offset < 0)
        return -1;

    int result = delete_key(bt, bt->root_offset, key);

    /* Check if root became empty (non-leaf) after deletion */
    DiskNode root;
    if (dm_read_node(bt->dm, bt->root_offset, &root) == 0) {
        if (root.num_keys == 0 && !root.is_leaf) {
            bt->root_offset = root.children[0];
            bt->dm->header.root_offset = bt->root_offset;
            dm_write_header(bt->dm);
        } else if (root.num_keys == 0 && root.is_leaf) {
            bt->root_offset = -1;
            bt->dm->header.root_offset = -1;
            dm_write_header(bt->dm);
        }
    }

    return result;
}
