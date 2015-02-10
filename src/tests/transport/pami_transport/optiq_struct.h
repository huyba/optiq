#ifndef OPTIQ_STRUCT_H
#define OPTIQ_STRUCT_H

#include <pami.h>

struct optiq_memregion {
    pami_memregion_t mr;
    int offset;
    int header_id;
};

struct optiq_message_header {
    int length;
    int source;
    int dest;
    int path_id;
    struct optiq_memregion mem;
    int header_id;
    int original_offset;
};

#endif
