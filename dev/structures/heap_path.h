#ifndef OPTIQ_HEAP_PATH
#define OPTIQ_HEAP_PATH

#include "path.h"
#include "heap_path.h"

struct heap_path {
    int num_elements;
    struct path **heap;
};

void hp_create(struct heap_path *hp, int max_num_elements);

void hp_destroy(struct heap_path *hp);

void hp_insert(struct heap_path *hp, struct path *new_path);

void hp_shift_up(struct heap_path *hp, int index);

void hp_shift_down(struct heap_path *hp, int index);

struct path* hp_find_min(struct heap_path *hp);

void hp_remove_min(struct heap_path *hp);

void hp_heapify(struct heap_path *hp, int index);

void hp_print(struct heap_path *hp);

#endif
