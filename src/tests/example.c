#include <stdio.h>
#include <stdlib.h>

#include <optiq.h>

int main(int argc, char **argv)
{
    optiq_init();

    optiq_get_topology_from_file("topo_edison_128");
    void *buf1, *buf2;
    optiq_add_request(0, 3, buf1, 1024, OPTIQ_IO);
    optiq_add_request(0, 3, buf2, 1024, OPTIQ_COMM);
    optiq_generate_model_data("optiq.dat");

    optiq_optimize();

    optiq_transfer();

    return 0;
}
