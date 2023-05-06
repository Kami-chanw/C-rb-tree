// #include "mapreduce.h"
#include "../include/rb_tree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int comp(void* a, void* b) {
    int l = *(int*)a, r = *(int*)b;
    if (l < r)
        return -1;
    if (l == r)
        return 0;
    if (l > r)
        return 1;
}

void dtor(void*) {}

void print_tree(const char* str){
    printf(str);
}

void print_val(void* val) {
    printf("%d", *(int*)val);
}
int main() {
    rbt_tree* tree  = rbt_create(comp);
    int       arr[] = { 10, 1, 5, 15, 2, 100 };
    for (int i = 0; i < 5; ++i)
        rbt_insert(tree, arr + i);
    rbt_erase(tree, arr + 2, dtor);
    rbt_erase(tree, arr + 0, dtor);
    rbt_for_each_val(tree, int*, val) { printf("%d ", *val); }
    printf("\n");
    rbt_display(tree, print_tree, print_val);
    rbt_destroy(tree, dtor);
}