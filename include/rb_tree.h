#ifndef _RB_TREE_H
#define _RB_TREE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
// a better abbreviation is rb_xxx but in order to prevent ambiguity, rbt_xxx is determined

typedef struct rbt_tree rbt_tree;
typedef struct node_t   node_t;

typedef struct {
    node_t*       node;
    unsigned long is_reverse;  // for reverse iterator
} rbt_iterator;

typedef struct {
    long         err;  // 0 for success, -1 for there is the same value exists.
    rbt_iterator pos;  // point to the new node or the already exist node
} rbt_insert_result_t;

typedef struct {
    long         err;
    rbt_iterator pos;
    void*        old;  // for assignment case, old value will be returned, NULL if insertion.
} rbt_insert_or_assign_result_t;

typedef struct {
    rbt_iterator suc;
    void*        value;
} rbt_erase_result_t;

typedef struct {
    rbt_iterator first;
    rbt_iterator last;
} rbt_eqrange_result_t;

typedef void (*rbt_val_dtor)(void*);
typedef int (*rbt_val_comp)(void*, void*);    // < 0 means less, == 0 means equal, > 0 means greater
typedef void (*rbt_tree_print)(const char*);  // print tree structure
typedef void (*rbt_val_print)(void*);         // print value

rbt_tree* rbt_create(rbt_val_comp cmpr);
void      rbt_destroy(rbt_tree*, rbt_val_dtor dtor);

rbt_insert_result_t           rbt_insert(rbt_tree*, void*);
rbt_insert_result_t           rbt_insert_unique(rbt_tree*, void*);
rbt_insert_or_assign_result_t rbt_insert_or_assign(rbt_tree*, void*);

void* rbt_extract(rbt_tree* tree, rbt_iterator it);

size_t       rbt_erase(rbt_tree*, void* value, rbt_val_dtor dtor);
rbt_iterator rbt_erase_at(rbt_tree*, rbt_iterator position, rbt_val_dtor dtor);
rbt_iterator rbt_erase_range(rbt_tree*, rbt_iterator first, rbt_iterator last, rbt_val_dtor dtor);
void         rbt_clear(rbt_tree*, rbt_val_dtor dtor);

rbt_iterator rbt_find(rbt_tree*, void*);
void*        rbt_val_at(rbt_tree*, void*);
void*        rbt_val_at_or(rbt_tree*, void*, void*);
size_t       rbt_size(rbt_tree*);

rbt_iterator         rbt_lower_bound(rbt_tree*, void*);
rbt_iterator         rbt_upper_bound(rbt_tree*, void*);
rbt_eqrange_result_t rbt_eqaul_range(rbt_tree*, void*);

rbt_iterator rbt_begin(rbt_tree* tree);
rbt_iterator rbt_end(rbt_tree* tree);
rbt_iterator rbt_rbegin(rbt_tree* tree);
rbt_iterator rbt_rend(rbt_tree* tree);

bool rbt_is_empty(rbt_tree* tree);

void rbt_display(rbt_tree* tree, rbt_tree_print, rbt_val_print);

rbt_iterator rbt_iter_next(rbt_iterator it);
rbt_iterator rbt_iter_prev(rbt_iterator it);
void*        rbt_iter_val(rbt_iterator it);
bool         rbt_iter_eq(rbt_iterator lhs, rbt_iterator rhs);
bool         rbt_iter_neq(rbt_iterator lhs, rbt_iterator rhs);

#define rbt_for_each_impl(tree, iter, ...)                                                                     \
    for (rbt_iterator iter = rbt_##__VA_ARGS__##begin(tree); rbt_iter_neq(iter, rbt_##__VA_ARGS__##end(tree)); \
         iter              = rbt_iter_next(iter))
#define rbt_for_each(tree, iter) rbt_for_each_impl(tree, iter)
#define rbt_for_each_r(tree, iter) rbt_for_each_impl(tree, iter, r)

#define rbt_for_each_val_impl(tree, type, val, line, ...) /*do not use it directly*/                                           \
    rbt_iterator __iter##line = rbt_##__VA_ARGS__##begin(tree);                                                                \
    for (type val; rbt_iter_neq(__iter##line, rbt_##__VA_ARGS__##end(tree)) && (val = (type)rbt_iter_val(__iter##line), true); \
         __iter##line = rbt_iter_next(__iter##line))
#define rbt_for_each_val(tree, type, val) rbt_for_each_val_impl(tree, type, val, __LINE__)
#define rbt_for_each_val_r(tree, type, val) rbt_for_each_val_impl(tree, type, val, __LINE__, r)

#define rbt_for_until_impl(tree, iter, val, ...)                                                       \
    for (rbt_iterator iter = rbt_##__VA_ARGS__##begin(tree);                                           \
         rbt_iter_neq(iter, rbt_##__VA_ARGS__##end(tree)) && tree->comp(rbt_iter_val(iter), val) != 0; \
         iter = rbt_iter_next(iter))
#define rbt_for_until(tree, iter) rbt_for_until_impl(tree, iter)
#define rbt_for_until_r(tree, iter) rbt_for_until_impl(tree, iter, r)

#define rbt_for_until_val_impl(tree, type, val, target, line, ...) /*do not use it directly*/                              \
    rbt_iterator __iter##line = rbt_##__VA_ARGS__##begin(tree);                                                            \
    for (type val; rbt_iter_neq(__iter##line, rbt_##__VA_ARGS__##end(tree)) && tree->comp(rbt_iter_val(iter), target) != 0 \
                   && (val = (type)rbt_iter_val(__iter##line), true);                                                      \
         __iter##line = rbt_iter_next(__iter##line))                                                                                                                           ); iter = rbt_iter_next(iter))
#define rbt_for_until_val(tree, type, val, target) rbt_for_until_val_impl(tree, type, val, target, __LINE__)
#define rbt_for_until_val_r(tree, type, val, target) rbt_for_until_val_impl(tree, type, val, target, __LINE__, r)
#ifdef __cplusplus
}
#endif

#endif /* _RB_TREE_H */