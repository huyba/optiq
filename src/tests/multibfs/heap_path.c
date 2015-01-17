#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "path.h"
#include "heap_path.h"

void hp_create(struct heap_path *hp, int max_num_elements)
{
    hp->num_elements = 0;
    hp->heap = (struct path **) malloc (sizeof(struct path*) * max_num_elements);
}

void hp_insert(struct heap_path *hp, struct path *new_path)
{
    hp->heap[hp->num_elements] = new_path;
    hp->num_elements++;
    new_path->hpos = hp->num_elements - 1;

    hp_shift_up(hp, hp->num_elements - 1);
}

void swap(struct heap_path *hp, int i1, int i2)
{
    struct path *temp;
    temp = hp->heap[i1];
    hp->heap[i1] = hp->heap[i2];
    hp->heap[i2] = temp;

    hp->heap[i1]->hpos = i2;
    hp->heap[i2]->hpos = i1;
}

void hp_shift_up(struct heap_path *hp, int index)
{
    if (index != 0) {
	if (hp->heap[index]->max_load < hp->heap[index/2]->max_load) 
	{
	    swap(hp, index, index/2);

	    hp_shift_up(hp, index/2);
	}
    }

    return;
}

void hp_shift_down(struct heap_path *hp, int index)
{
    /*If there are 2 children*/
    if (index *2 < hp->num_elements - 2) 
    {
	if ((hp->heap[index]->max_load > hp->heap[index * 2 + 1]->max_load) || 
		(hp->heap[index]->max_load > hp->heap[index * 2 + 2]->max_load)) 
	{
	    if (hp->heap[index * 2 + 1]->max_load < hp->heap[index * 2 + 2]->max_load) 
	    {
		swap(hp, index, index * 2 + 1);
		hp_shift_down(hp, index * 2 + 1);
	    } 
	    else 
	    {
		swap(hp, index, index * 2 + 2);
		hp_shift_down(hp, index * 2 + 2);
	    }
	}
    }
    /*If there is only one child*/
    else if (index * 2 < hp->num_elements - 1) 
    {
	if (hp->heap[index]->max_load > hp->heap[index * 2 + 1]->max_load) 
	{
	    swap(hp, index, index * 2 + 1);
	    hp_shift_down(hp, index * 2 + 1);
	}
    }

    return;
}

struct path* hp_find_min(struct heap_path *hp)
{
    return hp->heap[0];
}

void hp_remove_min(struct heap_path *hp)
{
    hp->heap[0] = hp->heap[hp->num_elements - 1];
    hp_shift_down(hp, 0);

    hp->num_elements--;   
}

void hp_heapify(struct heap_path *hp, int index)
{
    hp_shift_up(hp, index);
    hp_shift_down(hp, index);
}

void hp_print(struct heap_path *hp)
{
    printf("Print heap: num_elements = %d\n", hp->num_elements);

    for (int i = 0; i < hp->num_elements; i++) 
    {
	printf("path: %d max_load = %d, num_hops = %ld \n", i, hp->heap[i]->max_load, hp->heap[i]->arcs.size());
	for (int j = 0; j < hp->heap[i]->arcs.size(); j++)
	{
	    printf("%d->", hp->heap[i]->arcs[j].u);
	}
	printf("%d\n", hp->heap[i]->arcs.back().v);
    }
}
