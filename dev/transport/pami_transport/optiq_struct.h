#ifndef OPTIQ_STRUCT_H
#define OPTIQ_STRUCT_H

#include <pami.h>

struct optiq_memregion {
    pami_memregion_t mr;    /* PAMI memory region structure */
    int offset;             /* Offset of data in the memregion mr */
    int header_id;          /* Header id: used to keep track of memregion request and return. The local side uses this in the request. The remote side return a memregion with this id. */
};

struct optiq_message_header {
    int length;                 /* Length of the message */
    int source;                 /* Original source of the message - maybe not the previous/last source */
    int dest;                   /* Final destination of the message - maybe not the next destination */
    int path_id;                /* Path Id of the message. This is to determine the next destination at an intermediate node */
    struct optiq_memregion mem; /* Mem region of the message */
    int header_id;              /* Header id of the message, used to keep track of message mem when exchanging */
    int original_offset;        /* Offset of the message in the sender's buffer. So that when a receiver receives it, it know where to assemble the message */
};

#endif
