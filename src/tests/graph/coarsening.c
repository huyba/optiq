#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>

#include <mpi.h>

#include "optiq.h"

int main(int argc, char **argv)
{
    vector<struct optiq_job> jobs;

    const char *filePath = "../../../model/flow32";
    if (argc > 1) {
	filePath = argv[1];
    }
    optiq_job_read_from_file(filePath, &jobs);

    optiq_job_print(&jobs);

    int size[5] = {2,4,4,4,2};
    int dims = 5;

    return 0;
}
