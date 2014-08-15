#include "route.h"

#define MAX_ARCS 1024
#define MAX_ROUTES 16384

void readRoute(char *filePath) {
    char line[256];
    FILE *fr = fopen(filePath, "rt");
    int source, dest;
    float load;

    int num_sources = 256;
    int max_route = 10;

    arc_t *arcs = (arc_t*)malloc(sizeof(arc_t)*MAX_ARCS);
    route_t *routes = (route_t*)malloc(sizeof(route_t)*MAX_ROUTES);

    for(int i = 0; i < 5; i++)
	fgets(line, 256, fr);

    int num_arcs = 0;
    int current_source = 0;
    while(fgets(line, 256, fr) != NULL) {
	if(line[1] == '\n') {
	    num_arcs = 0;
	}
	if(line[0] != '\n') {
	    sscanf(line, "%d %d %f", &source, &dest, &load);
	    arcs[0].source = source;
	    arcs[0].dest = dest;
	    arcs[0].load = load;
	    num_arcs++;
	}
	
    }

    fclose(fr);
}

void main(int argc, char **argv) {
    char *filePath;
    if(argc > 1)
	filePath = argv[1];
    else {
	printf("No file name!\n");
	return;
    }

    readRoute(filePath);
}
