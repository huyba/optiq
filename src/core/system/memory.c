#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "memory.h"

void* core_memory_pool_allocate(int size)
{
    void *buffer = NULL;

    return buffer;
}

void* core_memory_alloc(int size, string var_name, string function_name)
{
    void *buffer = malloc(size);

    if (buffer == NULL) {
        char error_message[256];
        sprintf(error_message, "Error in allocating memory of %d bytes for variable %s in function %s", size, var_name.c_str(), function_name.c_str());

        printf("%s\n", error_message);
        exit(0);
    }

    return buffer;
}
