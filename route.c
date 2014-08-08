#include "route.h"

void readRoute(char *filePath) {
    char line[80];
    FILE *fr = fopen(filePath, "rt");
    int source, dest;
    float load;

    int num_sources = 256;
    int max_route = 10;

    for(int i = 0; i < 5; i++)
	fgets(line, 80, fr);

    printf("Hello world\n");

    while(fgets(line, 80, fr) != NULL) {
	while(line[0] == '\n') {
	    fgets(line, 80, fr);
	}

	sscanf(line, "%d %d %f", &source, &dest, &load);
	printf("%d %d %f\n", source, dest, load);
    }

    fclose(fr);
}

void main(int argc, char **argv) {
    char filePath[] = "flow32";
    readRoute(filePath);
}
