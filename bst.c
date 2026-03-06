/*
 * bst.c
 * In-memory Binary Search Tree implementation.
 */

#include "bst.h"
#include <stdlib.h>
#include <stdio.h>

/* ------------------------------------------------------------------ */
/*  Insert                                                             */
/* ------------------------------------------------------------------ */
void bst_insert(BSTNode **root, int key)
{
    if (*root == NULL) {
        BSTNode *node = (BSTNode *)malloc(sizeof(BSTNode));
        if (!node) {
            fprintf(stderr, "[bst] Memory allocation failed.\n");
            return;
        }
        node->key   = key;
        node->left  = NULL;
        node->right = NULL;
        *root = node;
        return;
    }

    if (key < (*root)->key)
        bst_insert(&(*root)->left, key);
    else if (key > (*root)->key)
        bst_insert(&(*root)->right, key);
    /* else: duplicate — silently ignored */
}

/* ------------------------------------------------------------------ */
/*  Search                                                             */
/* ------------------------------------------------------------------ */
int bst_search(BSTNode *root, int key)
{
    while (root) {
        if (key == root->key)
            return 1;
        else if (key < root->key)
            root = root->left;
        else
            root = root->right;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Delete                                                             */
/* ------------------------------------------------------------------ */

/* Find the in-order successor (smallest in right subtree) */
static BSTNode *find_min(BSTNode *node)
{
    while (node->left)
        node = node->left;
    return node;
}

BSTNode *bst_delete(BSTNode *root, int key)
{
    if (!root) return NULL;

    if (key < root->key) {
        root->left = bst_delete(root->left, key);
    } else if (key > root->key) {
        root->right = bst_delete(root->right, key);
    } else {
        /* Found the node to delete */
        if (!root->left) {
            BSTNode *right = root->right;
            free(root);
            return right;
        }
        if (!root->right) {
            BSTNode *left = root->left;
            free(root);
            return left;
        }

        /* Two children — replace with in-order successor */
        BSTNode *successor = find_min(root->right);
        root->key   = successor->key;
        root->right = bst_delete(root->right, successor->key);
    }
    return root;
}

/* ------------------------------------------------------------------ */
/*  Destroy                                                            */
/* ------------------------------------------------------------------ */
void bst_destroy(BSTNode *root)
{
    if (!root) return;
    bst_destroy(root->left);
    bst_destroy(root->right);
    free(root);
}
