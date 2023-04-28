## Structures

### rbt_tree

```c
typedef struct rbt_tree rbt_tree;
```

A struct representing a red-black tree data structure.

### node_t

```c
typedef struct node_t node_t;
```

A struct representing a node in the red-black tree.

### rbt_iterator

```c
typedef struct {
    node_t*       node;
    unsigned long is_reverse;
} rbt_iterator;
```

A struct representing an iterator used to traverse the red-black tree.

- `node`: a pointer to the current node pointed to by the iterator.
- `is_reverse`: a flag indicating whether the iterator is in reverse mode or not.

### rbt_insert_result_t

```c
typedef struct {
    long         err;
    rbt_iterator pos;
} rbt_insert_result_t;
```

A struct representing the result of a red-black tree insert operation.

- `err`: an error code, 0 for success and -1 if there is the same value exists.
- `pos`: a pointer to the new node or the already exist node.

### rbt_insert_or_assign_result_t

```c
typedef struct {
    long         err;
    rbt_iterator pos;
    void*        old;
} rbt_insert_or_assign_result_t;
```

A struct representing the result of a red-black tree insert or assign operation.

- `err`: an error code, 0 for success and -1 if there is the same value exists.
- `pos`: a pointer to the new node or the already exist node.
- `old`: for assignment case, the old value will be returned, NULL if insertion.

### rbt_erase_result_t

```c
typedef struct {
    rbt_iterator suc;
    void*        value;
} rbt_erase_result_t;
```

A struct representing the result of a red-black tree erase operation.

- `suc`: an iterator pointing to the successor node.
- `value`: the value of the erased node.

### rbt_eqrange_result_t

```c
typedef struct {
    rbt_iterator first;
    rbt_iterator last;
} rbt_eqrange_result_t;
```

A struct representing the result of a red-black tree equal range operation.

- `first`: an iterator pointing to the first node in the range.
- `last`: an iterator pointing to the last node in the range.

## Functions

### rbt_create

```c
rbt_tree* rbt_create(rbt_val_comp cmpr);
```

Creates a new red-black tree with the given comparator.

- `cmpr`: a function pointer used to compare values.

Returns a pointer to the newly created red-black tree.

### rbt_destroy

```c
void rbt_destroy(rbt_tree* tree, rbt_val_dtor dtor);
```

Destroys the given red-black tree and frees any allocated memory.

- `tree`: a pointer to the red-black tree to be destroyed.
- `dtor`: a function pointer used to free memory associated with values in the tree.

### rbt_insert

```c
rbt_insert_result_t rbt_insert(rbt_tree* tree, void* value);
```

Inserts a value into the red-black tree.

- `tree`: a pointer to the red-black tree to insert the value into.
- `value`: a pointer to the value to be inserted.

Returns a `rbt_insert_result_t` struct representing the result of the insert operation.

### rbt_insert_unique

```c
rbt_insert_result_t rbt_insert_unique(rbt_tree* tree, void* value);
```

This function inserts a new node with the specified `value` into the red-black tree `tree`, only if there is no node with an equal value already in the tree. The function returns a `rbt_insert_result_t` struct that contains the following fields:

- `err`: A flag that indicates if the insertion was successful. It will be 0 if the value was successfully inserted, or -1 if there is already a node with the same value in the tree.
- `pos`: An iterator that points to the node that was inserted, or the node that prevented the insertion if the value was not inserted.

### rbt_insert_or_assign

```c
rbt_insert_or_assign_result_t rbt_insert_or_assign(rbt_tree* tree, void* value);
```

This function inserts a new node with the specified `value` into the red-black tree `tree`, or assigns the value to an existing node with an equal value. The function returns a `rbt_insert_or_assign_result_t` struct that contains the following fields:

- `err`: A flag that indicates if the insertion or assignment was successful. It will be 0 if the value was successfully inserted or assigned, or -1 if an error occurred during insertion or assignment.
- `pos`: An iterator that points to the node that was inserted or assigned.
- `old`: A pointer to the old value if the value was assigned to an existing node, or NULL if the value was inserted as a new node.

### rbt_extract

```c
void* rbt_extract(rbt_tree* tree, rbt_iterator it);
```

This function removes the node pointed to by the specified iterator `it` from the red-black tree `tree`, and returns a pointer to the value that was stored in the node. The caller is responsible for freeing the memory associated with the returned value if necessary. If the iterator `it` does not point to a valid node in the tree, the function returns NULL.

### rbt_erase

```c
size_t rbt_erase(rbt_tree* tree, void* value, rbt_val_dtor dtor);
```

This function removes all nodes with the specified `value` from the red-black tree `tree`, and returns the number of nodes that were removed. The caller can optionally provide a `rbt_val_dtor` function `dtor` that will be called on each removed value to free any associated memory. If the caller does not need to free any memory, the `dtor` argument can be set to NULL.

### rbt_erase_at

```c
rbt_iterator rbt_erase_at(rbt_tree* tree, rbt_iterator position, rbt_val_dtor dtor);
```

This function removes the node pointed to by the specified iterator `position` from the red-black tree `tree`, and returns an iterator that points to the next node in the tree after the removed node. The caller can optionally provide a `rbt_val_dtor` function `dtor` that will be called on the value of the removed node to free any associated memory. If the caller does not need to free any memory, the `dtor` argument can be set to NULL.

### rbt_erase_range

```c
rbt_iterator rbt_erase_range(rbt_tree* tree, rbt_iterator first, rbt_iterator last, rbt_val_dtor dtor);
```

This function removes all nodes in the range `[first, last)` from the red-black tree `tree`, and returns an iterator that points to the next node in the tree after the last removed node. The caller can optionally provide a `rbt_val_dtor` function `dtor` that will be called on the value of each removed node to free any dynamically allocated memory associated with it.

#### Parameters
- `tree`: A pointer to the red-black tree from which the nodes are to be removed.
- `first`: A `rbt_iterator` pointing to the first node to be removed.
- `last`: A `rbt_iterator` pointing to the position just after the last node to be removed.
- `dtor`: A pointer to a function that takes a `void*` argument and frees any dynamically allocated memory associated with it. If `dtor` is `NULL`, the values will not be destroyed.

#### Return Value
This function returns an iterator that points to the next node in the tree after the last removed node.

### rbt_clear
```c
void rbt_clear(rbt_tree* tree, rbt_val_dtor dtor);
```
This function removes all nodes from the red-black tree `tree`, and optionally frees any dynamically allocated memory associated with the values using the `dtor` function.

#### Parameters
- `tree`: A pointer to the red-black tree to be cleared.
- `dtor`: A pointer to a function that takes a `void*` argument and frees any dynamically allocated memory associated with it. If `dtor` is `NULL`, the values will not be destroyed.

#### Return Value
This function does not return a value.

### rbt_find
```c
rbt_iterator rbt_find(rbt_tree* tree, void* value);
```
This function searches the red-black tree `tree` for a node with the given value `value`, and returns an iterator to the node if it is found, or an end iterator if it is not.

#### Parameters
- `tree`: A pointer to the red-black tree to be searched.
- `value`: A pointer to the value to be searched for.

#### Return Value
This function returns a `rbt_iterator` that points to the node with the given value `value` if it is found, or an end iterator if it is not.

### rbt_val_at
```c
void* rbt_val_at(rbt_tree* tree, void* value);
```
This function searches the red-black tree `tree` for a node with the given value `value`, and returns a pointer to the value of the node if it is found, or `NULL` if it is not.

#### Parameters
- `tree`: A pointer to the red-black tree to be searched.
- `value`: A pointer to the value to be searched for.

#### Return Value
This function returns a pointer to the value of the node with the given value `value` if it is found, or `NULL` if it is not.

### rbt_val_at_or
```c
void* rbt_val_at_or(rbt_tree* tree, void* value, void* default_value);
```
This function searches the red-black tree `tree` for a node with the given value `value`, and returns a pointer to the value of the node if it is found, or `default_value` if it is not.

#### Parameters
- `tree`: A pointer to the red-black tree to be searched.
- `value`: A pointer to the value to be searched for.
- `default_value`: A pointer to the default value to be returned if the node is not found.

#### Return Value
This function returns a pointer to the value of the node with the given value `value` if it is found, or `default_value` if it is not.

### rbt_size
```c
size_t rbt_size(rbt_tree* tree);
```
This function returns the number of nodes in the red-black tree `tree`.

#### Parameters
- `tree`: A pointer to the red-black tree to count the number of nodes in.

#### Return Value
The number of nodes in the red-black tree.

### rbt_lower_bound

```c
rbt_iterator rbt_lower_bound(rbt_tree* tree, void* value);
```

This function returns an iterator pointing to the first element in the red-black tree `tree` that is not less than `value`.

#### Parameters
- `tree`: A pointer to the red-black tree to search in.
- `value`: The value to search for.

#### Return Value
An iterator pointing to the first element in the red-black tree that is not less than `value`.

### rbt_upper_bound

```c
rbt_iterator rbt_upper_bound(rbt_tree* tree, void* value);
```

This function returns an iterator pointing to the first element in the red-black tree `tree` that is greater than `value`.

#### Parameters
- `tree`: A pointer to the red-black tree to search in.
- `value`: The value to search for.

#### Return Value
An iterator pointing to the first element in the red-black tree that is greater than `value`.

### rbt_equal_range

```c
rbt_eqrange_result_t rbt_equal_range(rbt_tree* tree, void* value);
```

This function returns a range of iterators representing the elements in the red-black tree `tree` that have a value equal to `value`.

#### Parameters
- `tree`: A pointer to the red-black tree to search in.
- `value`: The value to search for.

#### Return Value
An `rbt_eqrange_result_t` struct containing two iterators that represent the range of elements in the red-black tree that have a value equal to `value`.

### rbt_begin

```c
rbt_iterator rbt_begin(rbt_tree* tree);
```

This function returns an iterator pointing to the first element in the red-black tree `tree`.

#### Parameters
- `tree`: A pointer to the red-black tree.

#### Return Value
An iterator pointing to the first element in the red-black tree.

### rbt_end

```c
rbt_iterator rbt_end(rbt_tree* tree);
```

This function returns an iterator pointing to the past-the-end element in the red-black tree `tree`.

#### Parameters
- `tree`: A pointer to the red-black tree.

#### Return Value
An iterator pointing to the past-the-end element in the red-black tree.

### rbt_rbegin

```c
rbt_iterator rbt_rbegin(rbt_tree* tree);
```

This function returns a reverse iterator pointing to the last element in the red-black tree `tree`.

#### Parameters
- `tree`: A pointer to the red-black tree.

#### Return Value
A reverse iterator pointing to the last element in the red-black tree.

### rbt_rend

```c
rbt_iterator rbt_rend(rbt_tree* tree);
```

This function returns a reverse iterator pointing to the element preceding the first element in the red-black tree `tree`.

#### Parameters
- `tree`: A pointer to the red-black tree.

#### Return Value
A reverse iterator pointing to the element preceding the first element in the red-black tree.

### rbt_is_empty

```c
bool rbt_is_empty(rbt_tree* tree);
```

This function returns `true` if the red-black tree `tree` is empty (i.e., contains no nodes), and `false` otherwise.

#### Parameters
- `tree`: A pointer to the red-black tree.

#### Return Value
`true` if the red-black tree is empty, and `false` otherwise.

### rbt_iter_next

```c
rbt_iterator rbt_iter_next(rbt_iterator it);
```

This function returns an iterator pointing to the element following the element pointed to by the iterator `it`.

#### Parameters
- `rbt_iterator it`: An iterator pointing to an element in the red-black tree.

#### Return Value
An iterator pointing to the element following the element pointed to by the iterator `it`. If `it` is the last element in the red-black tree, the returned iterator is equal to the end iterator.

### rbt_iter_prev

```c
rbt_iterator rbt_iter_prev(rbt_iterator it);
```

This function returns an iterator pointing to the element preceding the element pointed to by the iterator `it`.

#### Parameters
- `rbt_iterator it`: An iterator pointing to an element in the red-black tree.

#### Return Value
An iterator pointing to the element preceding the element pointed to by the iterator `it`. If `it` is the first element in the red-black tree, the returned iterator is equal to the rend iterator.

### rbt_iter_val

```c
void* rbt_iter_val(rbt_iterator it);
```

This function returns a pointer to the value stored in the node pointed to by the iterator `it`.

#### Parameters
- `rbt_iterator it`: An iterator pointing to an element in the red-black tree.

#### Return Value
A pointer to the value stored in the node pointed to by the iterator `it`.

### rbt_iter_eq

```c
bool rbt_iter_eq(rbt_iterator lhs, rbt_iterator rhs);
```

This function compares two iterators for equality.

#### Parameters
- `rbt_iterator lhs`: The first iterator to compare.
- `rbt_iterator rhs`: The second iterator to compare.

#### Return Value
`true` if the iterators point to the same element in the red-black tree, `false` otherwise.

### rbt_iter_neq

```c
bool rbt_iter_neq(rbt_iterator lhs, rbt_iterator rhs);
```

This function compares two iterators for inequality.

#### Parameters
- `rbt_iterator lhs`: The first iterator to compare.
- `rbt_iterator rhs`: The second iterator to compare.

#### Return Value
`true` if the iterators point to different elements in the red-black tree, `false` otherwise.

### rbt_for_each / rbt_for_each_r

```c
#define rbt_for_each(tree, iter)
#define rbt_for_each_r(tree, iter)
```

These macro functions are used to travel the entire red-black tree forward or reversely

#### Parameters
- `tree`: The tree you want to travel
- `iter`: Name of iterator

### rbt_for_each_val / rbt_for_each_val_r

```c
#define rbt_for_each_val(tree, type, val)
#define rbt_for_each_val_r(tree, type, val)
```

These macro functions are used to travel the entire red-black tree forward or reversely and dereference iterator

#### Parameters
- `tree`: The tree you want to travel
- `type`: Type of node value, it should be a pointer
- `val` : Name of value

### rbt_for_until / rbt_for_until_r

```c
#define rbt_for_until(tree, iter, target)
#define rbt_for_until_r(tree, iter, target)
```

These macro functions are used to travel the entire red-black tree forward or reversely until find a specific value

#### Parameters
- `tree`: The tree you want to travel
- `iter`: Type of node value, it should be a pointer
- `target` : Name of value to find, if found target, loop will complete

### rbt_for_until_val / rbt_for_until_val_r

```c
#define rbt_for_until_val(tree, type, val, target)
#define rbt_for_until_val_r(tree, type, val, target)
```

These macro functions are used to travel the entire red-black tree forward or reversely until find a specific value and dereference iterator

#### Parameters
- `tree`: The tree you want to travel
- `type`: Type of node value, it should be a pointer
- `val` : Name of value
- `target` : Name of value to find, if found target, loop will complete

