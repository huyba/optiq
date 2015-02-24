#ifndef OPTIQ_MEMORY_H
#define OPTIQ_MEMORY_H

#include <string>

using namespace std;

void* core_memory_pool_allocate(int size);
void* core_memory_alloc(int size, string var_name, string function_name);

#endif
