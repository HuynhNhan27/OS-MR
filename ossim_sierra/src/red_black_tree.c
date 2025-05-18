#include "red_black_tree.h"


int createRBNode(struct RBTree *tree, struct pcb_t *data, struct RBNode **out_node) {
    if (!tree || !out_node) return -1;
    struct RBNode *node = (struct RBNode *)malloc(sizeof(struct RBNode));
    if (!node) return -1;
    node->data = data;
    node->color = RED;
    node->left = tree->TNULL;
    node->right = tree->TNULL;
    node->parent = NULL;
    *out_node = node;
    return 0;
}

int leftRotate(struct RBTree *tree, struct RBNode *x) {
    if (!tree || !x || x->right == tree->TNULL) return -1;
    struct RBNode *y = x->right;
    x->right = y->left;
    if (y->left != tree->TNULL)
        y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == NULL)
        tree->root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;
    y->left = x;
    x->parent = y;
    return 0;
}

int rightRotate(struct RBTree *tree, struct RBNode *x) {
    if (!tree || !x || x->left == tree->TNULL) return -1;
    struct RBNode *y = x->left;
    x->left = y->right;
    if (y->right != tree->TNULL)
        y->right->parent = x;
    y->parent = x->parent;
    if (x->parent == NULL)
        tree->root = y;
    else if (x == x->parent->right)
        x->parent->right = y;
    else
        x->parent->left = y;
    y->right = x;
    x->parent = y;
    return 0;
}

int insertFixup(struct RBTree *tree, struct RBNode *z) {
    if (!tree || !z) return -1;

    while (z->parent && z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            struct RBNode *y = z->parent->parent->right;
            if (y && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    leftRotate(tree, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rightRotate(tree, z->parent->parent);
            }
        } else {
            struct RBNode *y = z->parent->parent->left;
            if (y && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rightRotate(tree, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                leftRotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = BLACK;
    return 0;
}

int insert(struct RBTree *tree, struct pcb_t *data) {
    if (!tree || !data) return -1;

    struct RBNode *z = NULL;
    if (createRBNode(tree, data, &z) != 0) return -1;

    struct RBNode *y = NULL;
    struct RBNode *x = tree->root;

    while (x != tree->TNULL) {
        y = x;
        if (tree->cmp(z->data, x->data) < 0)
            x = x->left;
        else
            x = x->right;
    }

    z->parent = y;
    if (y == NULL)
        tree->root = z;
    else if (tree->cmp(z->data, y->data) < 0)
        y->left = z;
    else
        y->right = z;

    return insertFixup(tree, z);
}

int initializeRBTree(struct RBTree **tree, int (*compare)(struct pcb_t *, struct pcb_t *)) {
    if (!tree || !compare) {
        return -1;
    }

    *tree = (struct RBTree *)malloc(sizeof(struct RBTree));
    if (!*tree) {
        return -1;
    }

    (*tree)->TNULL = (struct RBNode *)malloc(sizeof(struct RBNode));
    if (!(*tree)->TNULL) {
        free(*tree);
        return -1;
    }

    (*tree)->TNULL->color = BLACK;
    (*tree)->TNULL->left = (*tree)->TNULL->right = (*tree)->TNULL->parent = NULL;
    (*tree)->TNULL->data = NULL;

    (*tree)->root = (*tree)->TNULL;
    (*tree)->cmp = compare;

    return 0;
}

struct RBNode* tree_minimum(struct RBTree* tree, struct RBNode* node) {
    if (!tree || node == tree->TNULL) return NULL;
    while (node->left != tree->TNULL) node = node->left;
    return node;
}

static int transplant(struct RBTree *tree, struct RBNode *u, struct RBNode *v) {
    if (!tree || !u) return -1;
    if (u->parent == NULL) {
        tree->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    if (v) v->parent = u->parent;
    return 0;
}

int remove_fixup(struct RBTree *tree, struct RBNode *x) {
    while (x != tree->root && x->color == BLACK) {
        if (x == x->parent->left) {
            struct RBNode *w = x->parent->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                leftRotate(tree, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    rightRotate(tree, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                leftRotate(tree, x->parent);
                x = tree->root;
            }
        } else {
            struct RBNode *w = x->parent->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rightRotate(tree, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    leftRotate(tree, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rightRotate(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = BLACK;
    return 0;
}

int _remove(struct RBTree *tree, struct RBNode *z) {
    if (!tree || !z || z == tree->TNULL) return -1;

    struct RBNode *y = z;
    struct RBNode *x = NULL;
    Color y_original_color = y->color;

    if (z->left == tree->TNULL) {
        x = z->right;
        transplant(tree, z, z->right);
    } else if (z->right == tree->TNULL) {
        x = z->left;
        transplant(tree, z, z->left);
    } else {
        y = tree_minimum(tree, z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            if (x) x->parent = y;
        } else {
            transplant(tree, y, y->right);
            y->right = z->right;
            if (y->right) y->right->parent = y;
        }
        transplant(tree, z, y);
        y->left = z->left;
        if (y->left) y->left->parent = y;
        y->color = z->color;
    }

    if (y_original_color == BLACK && x) {
        remove_fixup(tree, x);
    }

    free(z);
    return 0;
}

int insertRBTree(struct RBTree* tree, struct pcb_t* data) {
    int val = insert(tree, data);

    return val;
}

int removeminRBTree(struct RBTree *tree, struct pcb_t **data) {
    if (!tree || !data) return -1;


    struct RBNode* min_node = tree_minimum(tree, tree->root);
    if (!min_node || min_node == tree->TNULL) {
        return -1;
    }

    *data = min_node->data;
    int val = _remove(tree, min_node);

    return val;
}

int remove_by_search(struct RBTree* tree, int (*findFunc)(struct pcb_t*), struct pcb_t** data) {
    if (!tree || !findFunc || !data) return -1;

    struct RBNode* stack[100];
    int top = -1;
    struct RBNode* current = tree->root;

    if (current == tree->TNULL) return -1;

    stack[++top] = current;

    while (top >= 0) {
        current = stack[top--];

        if (current == tree->TNULL) continue;

        if (findFunc(current->data)) {
            *data = current->data;
            return _remove(tree, current);
        }

        if (current->right != tree->TNULL)
            stack[++top] = current->right;
        if (current->left != tree->TNULL)
            stack[++top] = current->left;
    }

    return -1;
}

int removeRBTree(struct RBTree* tree, int (*findFunc)(struct pcb_t*), struct pcb_t** data) {
    int val = remove_by_search(tree, findFunc, data);
    return val;
}

int traverseRBTree(struct RBTree* tree, void (*func)(struct pcb_t*)) {
    if (!tree || !func) return -1;

    struct RBNode* stack[100];
    int top = -1;
    struct RBNode* current = tree->root;

    if (current == tree->TNULL) return -1;

    stack[++top] = current;

    while (top >= 0) {
        current = stack[top--];

        if (current == tree->TNULL) continue;

        func(current->data);

        if (current->right != tree->TNULL)
            stack[++top] = current->right;
        if (current->left != tree->TNULL)
            stack[++top] = current->left;
    }

    return -1;
}