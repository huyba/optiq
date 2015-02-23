#include "optiq.h"

struct optiq *opq;

void optiq_init()
{
    opq = (struct optiq *)core_memory_alloc(sizeof(struct optiq), "optiq", "optiq_init");

    optiq_transport_init(&opq->transport, PAMI);
}

void optiq_create_communication_graph(int source, int dest, int nbytes, void *buffer)
{
    /*1. Create list of job*/
    struct optiq_job *job = (struct optiq_job *)core_memory_alloc(sizeof(struct optiq_job), "job", "create_communication_graph");
    job->source = source;
    job->dest = dest;
    job->demand = nbytes;
    job->buffer = (char *)buffer;

    /*2. Gather all jobs from all ranks*/

    /*2. Optimize the communication by an solver or write to a file*/

    /*3. Read the communication flows back*/
    

    /*4. Create virtual lanes, arbitration table */
}

void optiq_execute()
{

}
