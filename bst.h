/*
 * bst.h
 * In-memory Binary Search Tree with integer keys.
 * Used as a comparison baseline against the disk-backed B-Tree.
 */

#ifndef BST_H
#define BST_H

/* BST node */
typedef struct BSTNode {
    int             key;
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

/*
 * Insert `key` into the BST rooted at `*root`.
 * Duplicates are silently ignored.
 */
void bst_insert(BSTNode **root, int key);

/*
 * Search for `key` in the BST.
 * Returns 1 if found, 0 otherwise.
 */
int bst_search(BSTNode *root, int key);

/*
 * Delete `key` from the BST.
 * Returns the (possibly new) root.
 */
BSTNode *bst_delete(BSTNode *root, int key);

/*
 * Free all nodes in the BST.
 */
void bst_destroy(BSTNode *root);

#endif /* BST_H */
