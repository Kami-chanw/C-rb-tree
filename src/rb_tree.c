#include "rb_tree.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#define RED 0
#define BLACK 1

#define COLOR_OF(node) ((node)->color & 1)
#define IS_BLACK(node) (COLOR_OF(node) == BLACK)
#define IS_RED(node) (!IS_BLACK(node))

#define IS_NIL(node) (((node)->color & 2) != 0)
#define IS_LEFT(node) ((node) == (node)->parent->left)
#define IS_RIGHT(node) ((node) == (node)->parent->right)
#define IS_ACTUAL_ROOT(node) ((node) == (node)->parent->parent)

typedef struct node_t {
    void*          value;
    unsigned long  color;
    struct node_t* left;
    struct node_t* right;
    struct node_t* parent;
} node_t;

enum inspos { Left, Right };

typedef struct {
    node_t*     parent;
    enum inspos pos;
} ins_pack_t;

typedef struct {
    ins_pack_t pack;
    node_t*    curr;
} find_result_t;

typedef struct {  // for equal range
    node_t* first;
    node_t* second;
} nodeptr_pair_t;

typedef struct rbt_tree {
    node_t       root;
    size_t       size;
    rbt_val_comp comp;
} rbt_tree;

// basic operation
static node_t* leftmost(node_t* node);
static node_t* rightmost(node_t* node);
static node_t* inorder_predecessor(node_t* node);
static node_t* inorder_successor(node_t* node);
static void    set_color(node_t* node, int color);
static void    set_nil(node_t* node, bool is_nil);

// iterator operation
static node_t*      incr(node_t* node);
static node_t*      decr(node_t* node);
static rbt_iterator make_iter(node_t* node);
static rbt_iterator make_riter(node_t* node);

// tree implementation
static find_result_t  lower_bound(rbt_tree* tree, void*);
static find_result_t  upper_bound(rbt_tree* tree, void*);
static nodeptr_pair_t equal_range(rbt_tree* tree, void*);
static node_t*        create_node(rbt_tree* tree, void*);
static node_t*        insert_at(rbt_tree* tree, ins_pack_t pack, node_t* new_node);
static void           take_node(rbt_tree* tree, node_t* node, node_t* suc);
static node_t*        extract_node(rbt_tree* tree, node_t* node);  // extract node without free mem

// rb tree implementation
static void    insert_fixup(rbt_tree* tree, node_t* new_node);
static void    erase_fixup(rbt_tree* tree, node_t* node);
static node_t* rotate_left(rbt_tree* tree, node_t* node);
static node_t* rotate_right(rbt_tree* tree, node_t* node);

static void destroy(node_t* node, rbt_val_dtor dtor) {
    if (!IS_NIL(node)) {
        destroy(node->left, dtor);
        destroy(node->right, dtor);
        dtor(node->value);
        free(node);
    }
}

rbt_tree* rbt_create(rbt_val_comp comp) {
    rbt_tree* tree = (rbt_tree*)malloc(sizeof(rbt_tree));
    if (tree) {
        tree->comp       = comp;
        tree->size       = 0;
        tree->root.value = NULL;
        tree->root.color = 2;  // is nil and red
        tree->root.left = tree->root.right = tree->root.parent = &tree->root;
    }
    return tree;
}

void rbt_destroy(rbt_tree* tree, rbt_val_dtor dtor) {
    rbt_clear(tree, dtor);
    free(tree);
}

rbt_insert_result_t rbt_insert(rbt_tree* tree, void* value) {
    find_result_t res      = upper_bound(tree, value);
    int           ret      = ENOMEM;
    node_t*       new_node = create_node(tree, value);
    if (new_node) {
        res.curr = insert_at(tree, res.pack, new_node);
        ret      = 0;
    }
    return (rbt_insert_result_t){ .pos = make_iter(res.curr), .err = ret };
}

rbt_insert_result_t rbt_insert_unique(rbt_tree* tree, void* value) {
    find_result_t res = lower_bound(tree, value);
    int           ret = -1;
    if (IS_NIL(res.curr) || tree->comp(res.curr->value, value) != 0) {
        node_t* new_node = create_node(tree, value);
        if (new_node == NULL)
            ret = ENOMEM;
        else {
            res.curr = insert_at(tree, res.pack, new_node);
            ret      = 0;
        }
    }
    return (rbt_insert_result_t){ .pos = make_iter(res.curr), .err = ret };
}

rbt_insert_or_assign_result_t rbt_insert_or_assign(rbt_tree* tree, void* value) {
    find_result_t res = lower_bound(tree, value);
    int           ret = -1;
    void*         old = NULL;
    if (IS_NIL(res.curr) || tree->comp(res.curr->value, value) != 0) {
        node_t* new_node = create_node(tree, value);
        if (new_node == NULL)
            ret = ENOMEM;
        else {
            res.curr = insert_at(tree, res.pack, new_node);
            ret      = 0;
        }
    }
    else {
        old             = res.curr->value;
        res.curr->value = value;
    }
    return (rbt_insert_or_assign_result_t){ .pos = make_iter(res.curr), .err = ret, .old = old };
}

void* rbt_extract(rbt_tree* tree, rbt_iterator position) {
    node_t *curr = position.node;
    void* value = curr->value;  // the value dummy root is also NULL
    if (!IS_NIL(curr)) {
        node_t *suc = inorder_successor(curr);
        erase_fixup(tree, extract_node(tree, curr));
        free(curr);
        --tree->size;
    }
    return value;
}

size_t rbt_erase(rbt_tree* tree, void* value, rbt_val_dtor dtor) {
    nodeptr_pair_t res = equal_range(tree, value);
    size_t         n   = 0;

    for (; res.first != res.second; ++n) {
        node_t* suc = inorder_successor(res.first);
        rbt_erase_at(tree, make_iter(res.first), dtor);
        res.first = suc;
    }

    return n;
}

rbt_iterator rbt_erase_at(rbt_tree* tree, rbt_iterator position, rbt_val_dtor dtor) {
     node_t *curr = position.node;
    if (IS_NIL(curr))
        return rbt_end(tree);
    node_t *suc = inorder_successor(curr);
    dtor(curr->value);
    erase_fixup(tree, extract_node(tree, curr));
    free(curr);
    --tree->size;
    return make_iter(suc);
}

rbt_iterator rbt_erase_range(rbt_tree* tree, rbt_iterator first, rbt_iterator last, rbt_val_dtor dtor) {
    if (rbt_iter_eq(first, rbt_begin(tree)) && rbt_iter_eq(last, rbt_end(tree)))
        rbt_clear(tree, dtor);
    else {
        for (; rbt_iter_neq(first, last); first = rbt_iter_next(first))
            rbt_erase_at(tree, first, dtor);
        return make_iter(first.node);
    }
    return rbt_begin(tree);
}

void rbt_clear(rbt_tree* tree, rbt_val_dtor dtor) {
    destroy(tree->root.parent, dtor);
    tree->size      = 0;
    tree->comp      = NULL;
    tree->root.left = tree->root.right = tree->root.parent = &tree->root;
}

rbt_iterator rbt_find(rbt_tree* tree, void* key) {
    find_result_t res = lower_bound(tree, key);
    return (IS_NIL(res.curr) || tree->comp(key, res.curr->value) != 0) ? rbt_end(tree) : make_iter(res.curr);
}

void* rbt_val_at(rbt_tree* tree, void* value) {
    rbt_iterator res = rbt_find(tree, value);
    assert(res.node != &tree->root);
    return res.node->value;
}

void* rbt_val_at_or(rbt_tree* tree, void* value, void* default_val) {
    rbt_iterator res = rbt_find(tree, value);
    if (res.node == &tree->root)
        return default_val;
    return res.node->value;
}

size_t rbt_size(rbt_tree* tree) { return tree->size; }

rbt_iterator rbt_lower_bound(rbt_tree* tree, void* key) { return make_iter(lower_bound(tree, key).curr); }

rbt_iterator rbt_upper_bound(rbt_tree* tree, void* key) { return make_iter(upper_bound(tree, key).curr); }

rbt_eqrange_result_t rbt_eqaul_range(rbt_tree* tree, void* key) {
    nodeptr_pair_t res = equal_range(tree, key);
    return (rbt_eqrange_result_t){ make_iter(res.first), make_iter(res.second) };
}

rbt_iterator rbt_begin(rbt_tree* tree) { return make_iter(tree->root.left); }
rbt_iterator rbt_end(rbt_tree* tree) { return make_iter(&tree->root); }
rbt_iterator rbt_rbegin(rbt_tree* tree) { return make_riter(&tree->root); }
rbt_iterator rbt_rend(rbt_tree* tree) { return make_riter(tree->root.left); }

bool rbt_is_empty(rbt_tree* tree) { return tree->size == 0; }

rbt_iterator rbt_iter_next(rbt_iterator it) {
    return (rbt_iterator){ .is_reverse = it.is_reverse, .node = it.is_reverse ? decr(it.node) : incr(it.node) };
}
rbt_iterator rbt_iter_prev(rbt_iterator it) {
    return (rbt_iterator){ .is_reverse = it.is_reverse, .node = it.is_reverse ? incr(it.node) : decr(it.node) };
}
void* rbt_iter_val(rbt_iterator it) {
    if (it.is_reverse)
        return decr(it.node)->value;
    return it.node->value;
}
bool rbt_iter_eq(rbt_iterator lhs, rbt_iterator rhs) {
    assert(lhs.is_reverse == rhs.is_reverse);
    return lhs.node == rhs.node;
}
bool rbt_iter_neq(rbt_iterator lhs, rbt_iterator rhs) { return !rbt_iter_eq(lhs, rhs); }

static node_t* leftmost(node_t* node) {
    while (!IS_NIL(node->left))
        node = node->left;
    return node;
}

static node_t* rightmost(node_t* node) {
    while (!IS_NIL(node->right))
        node = node->right;
    return node;
}

static node_t* inorder_predecessor(node_t* node) {
    if (!IS_NIL(node->left))
        return rightmost(node->left);
    node_t* parent = node->parent;
    while (!IS_NIL(parent) && IS_LEFT(node)) {
        node   = parent;
        parent = node->parent;
    }
    if (!IS_NIL(node))
        node = parent;
    return node;
}

static node_t* inorder_successor(node_t* node) {
    if (!IS_NIL(node->right))
        return leftmost(node->right);
    while (!IS_NIL(node->parent) && IS_RIGHT(node))
        node = node->parent;
    return node->parent;
}

static void set_color(node_t* node, int color) {
    if (color == RED)
        node->color &= ~1;
    else
        node->color |= BLACK;
}
static void set_nil(node_t* node, bool is_nil) {
    if (is_nil)
        node->color |= 2;
    else
        node->color &= ~2;
}

static node_t* incr(node_t* node) { return inorder_successor(node); }

static node_t*      decr(node_t* node) { return IS_NIL(node) ? node->right : inorder_predecessor(node); }
static rbt_iterator make_iter(node_t* node) { return (rbt_iterator){ .node = node, .is_reverse = false }; }
static rbt_iterator make_riter(node_t* node) { return (rbt_iterator){ .node = node, .is_reverse = true }; }

static find_result_t lower_bound(rbt_tree* tree, void* key) {
    node_t*       curr = tree->root.parent;
    find_result_t res  = { .pack = { .parent = tree->root.parent }, .curr = &tree->root };
    while (!IS_NIL(curr)) {
        res.pack.parent = curr;
        if (tree->comp(curr->value, key) >= 0) {  // curr.key >= key
            res.pack.pos = Left;
            res.curr     = curr;
            curr         = curr->left;
        }
        else {
            res.pack.pos = Right;
            curr         = curr->right;
        }
    }
    return res;
}
static find_result_t upper_bound(rbt_tree* tree, void* key) {
    node_t*       curr = tree->root.parent;
    find_result_t res  = { .pack = { .parent = tree->root.parent }, .curr = &tree->root };
    while (!IS_NIL(curr)) {
        res.pack.parent = curr;
        if (tree->comp(curr->value, key) > 0) {  // curr.key > key
            res.pack.pos = Left;
            res.curr     = curr;
            curr         = curr->left;
        }
        else {
            res.pack.pos = Right;
            curr         = curr->right;
        }
    }
    return res;
}

nodeptr_pair_t equal_range(rbt_tree* tree, void* key) {
    node_t* root  = &tree->root;
    node_t *first = root, *second = root;
    node_t* curr = root->parent;
    while (!IS_NIL(curr))
        if (tree->comp(curr->value, key) < 0)
            curr = curr->right;
        else {
            if (IS_NIL(second) && tree->comp(key, curr->value) < 0)
                second = curr;
            first = curr;
            curr  = curr->left;
        }
    curr = IS_NIL(second) ? root->parent : second->left;
    while (!IS_NIL(curr))
        if (tree->comp(key, curr->value) < 0) {
            second = curr;
            curr   = curr->left;
        }
        else
            curr = curr->right;
    return (nodeptr_pair_t){ first, second };
}

static node_t* insert_at(rbt_tree* tree, ins_pack_t pack, node_t* new_node) {
    node_t* root     = &tree->root;
    new_node->parent = pack.parent;
    if (pack.parent == root)
        root->parent = root->left = root->right = new_node;
    else {
        if (pack.pos == Left) {
            pack.parent->left = new_node;
            if (pack.parent == root->left)
                root->left = new_node;
        }
        else {
            pack.parent->right = new_node;
            if (pack.parent == root->right)
                root->right = new_node;
        }
    }
    insert_fixup(tree, new_node);
    ++tree->size;
    return new_node;
}

static node_t* create_node(rbt_tree* tree, void* value) {
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    if (new_node) {
        assert(value);
        new_node->value = value;
        new_node->color = 0;
        new_node->left = new_node->parent = new_node->right = &tree->root;
    }
    return new_node;
}

static void take_node(rbt_tree* tree, node_t* node, node_t* suc) {
    if (IS_LEFT(node))
        node->parent->left = suc;
    else if (IS_RIGHT(node))
        node->parent->right = suc;
    if (!IS_NIL(suc))
        suc->parent = node->parent;
}

static node_t* extract_node(rbt_tree* tree, node_t* node) {
    node_t* root = &tree->root;
    node_t* suc  = NULL;
    if (root->left == node)
        suc = root->left = IS_NIL(node->right) ? node->parent : leftmost(node->right);
    if (root->right == node)
        suc = root->right = IS_NIL(node->left) ? node->parent : rightmost(node->left);
    if (root->parent == node)
        root->parent = suc ? suc : (IS_NIL(node->right) ? root : leftmost(node->right));

    node_t* fixnode;
    int     color = COLOR_OF(node);
    if (IS_NIL(node->left)) {
        fixnode = node->right;
        take_node(tree, node, node->right);
    }
    else if (IS_NIL(node->right)) {
        fixnode = node->left;
        take_node(tree, node, node->left);
    }
    else {
        suc     = leftmost(node->right);
        color   = COLOR_OF(suc);
        fixnode = suc->right;
        if (suc->parent == node)
            fixnode->parent = suc;
        else {
            take_node(tree, suc, suc->right);
            suc->right         = node->right;
            suc->right->parent = suc;
        }
        take_node(tree, node, suc);
        suc->left         = node->left;
        suc->left->parent = suc;
        set_color(suc, COLOR_OF(node));
    }
    return color == BLACK ? fixnode : tree->root.parent;
}

// rb tree implementation
static void insert_fixup(rbt_tree* tree, node_t* node) {
    node_t* uncle;
    while (!IS_ACTUAL_ROOT(node) && IS_RED(node->parent)) {
        if (node->parent == node->parent->parent->left) {
            uncle = node->parent->parent->right;
            if (IS_RED(uncle)) {
                set_color(uncle, BLACK);
                set_color(node->parent, BLACK);
                set_color(node->parent->parent, RED);

                node = node->parent->parent;
            }
            else {
                if (IS_RIGHT(node)) {
                    node = node->parent;
                    rotate_left(tree, node);
                }
                set_color(node->parent, BLACK);
                set_color(node->parent->parent, RED);
                rotate_right(tree, node->parent->parent);
            }
        }
        else {
            uncle = node->parent->parent->left;
            if (IS_RED(uncle)) {
                set_color(uncle, BLACK);
                set_color(node->parent, BLACK);
                set_color(node->parent->parent, RED);

                node = node->parent->parent;
            }
            else {
                if (IS_LEFT(node)) {
                    node = node->parent;
                    rotate_right(tree, node);
                }
                set_color(node->parent, BLACK);
                set_color(node->parent->parent, RED);
                rotate_left(tree, node->parent->parent);
            }
        }
    }
    set_color(tree->root.parent, BLACK);
}

static void erase_fixup(rbt_tree* tree, node_t* node) {
    node_t* bro;
    while (!IS_ACTUAL_ROOT(node) && IS_BLACK(node)) {
        if (IS_LEFT(node)) {
            bro = node->parent->right;
            if (IS_RED(bro)) {
                set_color(bro, BLACK);
                set_color(node->parent, RED);
                rotate_left(tree, node->parent);
                bro = node->parent->right;
            }
            if (IS_BLACK(bro->left) && IS_BLACK(bro->right)) {
                set_color(bro, RED);
                node         = node->parent;
                node->parent = node->parent->parent;
            }
            else {
                if (IS_BLACK(bro->right)) {
                    set_color(bro->left, BLACK);
                    set_color(bro, RED);
                    rotate_right(tree, bro);
                    bro = node->parent->right;
                }
                set_color(bro, COLOR_OF(node->parent));
                set_color(node->parent, BLACK);
                set_color(bro->right, BLACK);
                rotate_left(tree, node->parent);
                break;
            }
        }
        else {
            bro = node->parent->left;
            if (IS_RED(bro)) {
                set_color(bro, BLACK);
                set_color(node->parent, RED);
                rotate_right(tree, node->parent);
                bro = node->parent->left;
            }
            if (IS_BLACK(bro->right) && IS_BLACK(bro->left)) {
                set_color(bro, RED);
                node         = node->parent;
                node->parent = node->parent->parent;
            }
            else {
                if (IS_BLACK(bro->left)) {
                    set_color(bro->right, BLACK);
                    set_color(bro, RED);
                    rotate_left(tree, bro);
                    bro = node->parent->left;
                }
                set_color(bro, COLOR_OF(node->parent));
                set_color(node->parent, BLACK);
                set_color(bro->left, BLACK);
                rotate_right(tree, node->parent);
                break;
            }
        }
    }
    set_color(tree->root.parent, BLACK);
}
static node_t* rotate_left(rbt_tree* tree, node_t* node) {
    node_t* pivot = node->right;
    node->right   = pivot->left;
    if (!IS_NIL(pivot->left))
        pivot->left->parent = node;
    pivot->parent = node->parent;
    if (IS_ACTUAL_ROOT(node))
        tree->root.parent = pivot;
    else {
        if (IS_LEFT(node))
            node->parent->left = pivot;
        else
            node->parent->right = pivot;
    }

    pivot->left  = node;
    node->parent = pivot;
    return pivot;
}

static node_t* rotate_right(rbt_tree* tree, node_t* node) {
    node_t* pivot = node->left;
    node->left    = pivot->right;
    if (!IS_NIL(pivot->right))
        pivot->right->parent = node;
    pivot->parent = node->parent;
    if (IS_ACTUAL_ROOT(node))
        tree->root.parent = pivot;
    else {
        if (IS_RIGHT(node))
            node->parent->right = pivot;
        else
            node->parent->left = pivot;
    }
    pivot->right = node;
    node->parent = pivot;
    return pivot;
}
