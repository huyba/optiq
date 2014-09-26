#ifndef DATAGEN_H
#define DATAGEN_H

void optiq_print_arcs(int num_dims, int *size, double cap);
void optiq_generate_data(int num_dims, int *size);
void optiq_generate_dataIO(int num_dims, int *size, int num_sources, int factor, int num_bridges,  int *bridgeIds);

#endif
