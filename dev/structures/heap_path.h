/* 
 * A heap of path is used to contain paths and the first path being pick is at the top of the heap. The comparison function decides which one is at the top. 
 * */

#ifndef OPTIQ_HEAP_PATH
#define OPTIQ_HEAP_PATH

#include "path.h"
#include "heap_path.h"

/* Include the number of elements and header pointer */
struct heap_path {
    int num_elements;
    struct path **heap;
};

/* Create a heap */
void hp_create(struct heap_path *hp, int max_num_elements);

/* Destroy a heap */
void hp_destroy(struct heap_path *hp);

/* Insert a element in a heap */
void hp_insert(struct heap_path *hp, struct path *new_path);

/* Shift an element up */
void hp_shift_up(struct heap_path *hp, int index);

/* Shift an element down */
void hp_shift_down(struct heap_path *hp, int index);

/* Find the smallest element */
struct path* hp_find_min(struct heap_path *hp);

/* Remove the min element */
void hp_remove_min(struct heap_path *hp);

/* Doing heapify for entire heap */
void hp_heapify(struct heap_path *hp, int index);

/* Print all elements in a heap */
void hp_print(struct heap_path *hp);

#endif
