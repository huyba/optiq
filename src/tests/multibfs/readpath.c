#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>

#include <util.h>
#include <path.h>

using namespace std;

int main (int argc, char *argv[]) 
{
    char *filePath = argv[1];

    std::vector<struct path *> complete_paths;
    optiq_path_read_from_file(filePath, complete_paths);

    int num_nodes = 512;

    optiq_path_print_paths(complete_paths);
    optiq_path_print_stat(complete_paths, num_nodes);
}

