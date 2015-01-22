#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>

#include <util.h>
#include <path.h>

using namespace std;

int main (int argc, char *argv[]) 
{
    FILE * fp;
    char line[256];

    char *filePath = argv[1];

    fp = fopen(filePath, "r");
    if (fp == NULL) {
	exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 6; i++) 
    {
	fgets(line, 80, fp);
    }

    std::vector<struct path *> complete_paths;
    int job_id = 0, path_id = 0, u, v;
    char temp[256];

    while(fgets(line, 80, fp) != NULL)
    {
	//printf("%s", line);

	if (line[0] == 'J') 
	{
	    trim(line);
	    sscanf(line, "%s %d %d", temp, &job_id, &path_id);
	    //printf("job_id = %d path_id = %d\n", job_id, path_id);

	    struct path *p = (struct path *) calloc (1, sizeof(struct path));
	    p->job_id = job_id;
	    p->path_id = path_id;

	    while(fgets(line,80,fp) != NULL) 
	    {
		trim(line);
		if (strcmp(line, "") == 0)
		{
		    break;
		}
		sscanf(line, "%d %d", &u, &v);

		struct arc a;
		a.u = u;
		a.v = v;

		p->arcs.push_back(a);

		//printf("u = %d, v = %d\n", u, v);
	    }

	    complete_paths.push_back(p);
	}
    }

    int num_nodes = 256;

    optiq_path_print_paths(complete_paths);
    optiq_path_print_stat(complete_paths, num_nodes);

    fclose(fp);
}

