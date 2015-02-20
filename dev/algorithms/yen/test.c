#include "yen.h"

int main(int argc, char **argv)
{
    std::vector<struct path *> complete_paths;
    std::vector<std::pair<int, std::vector<int> > > source_dests;
    int num_paths = 3;
    char *graphFilePath = "";

    get_yen_k_shortest_paths (complete_paths, source_dests, num_paths, graphFilePath);
}
