#ifndef RED_BLACK_TREE_H
#define RED_BLACK_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include "common.h"

typedef enum { RED, BLACK } Color;

struct RBNode {
    struct pcb_t *data;
    Color color;
    struct RBNode *left, *right, *parent;
};

struct RBTree {
    int (*cmp)(struct pcb_t *, struct pcb_t *);
    struct RBNode *root;
    struct RBNode *TNULL; // Sentinel node
};

int initializeRBTree(struct RBTree **tree, int (*compare)(struct pcb_t *, struct pcb_t *));
int insertRBTree(struct RBTree *tree, struct pcb_t *data);
int removeminRBTree(struct RBTree *tree, struct pcb_t **data);
int removeRBTree(struct RBTree* tree, int (*findFunc)(struct pcb_t*), struct pcb_t** data);
int traverseRBTree(struct RBTree* tree, void (*func)(struct pcb_t*));
#endif // RED_BLACK_TREE_H