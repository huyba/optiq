#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <queue>

void rtrim(char *str)
{
    size_t n;
    n = strlen(str);
    while (n > 0 && isspace((unsigned char)str[n - 1])) {
	n--;
    }
    str[n] = '\0';
}
 
void ltrim(char *str)
{
    size_t n;
    n = 0;
    while (str[n] != '\0' && isspace((unsigned char)str[n])) {
	n++;
    }
    memmove(str, str + n, strlen(str) - n + 1);
}
 
void trim(char *str)
{
    rtrim(str);
    ltrim(str);
}

struct arc_t {
    int ep1;
    int ep2;
    int bw;
};

struct path {
    int num_arc;
    struct arct_t *arcs;
}

void printPath(int **arc, int n, int source, int dest) 
{
    std::queue<int> myqueue;
}

int main(int argc, char **argv)
{
    char *file_name = "flow85";
    FILE *file = fopen(file_name, "r");
 
    char buf[256];

    int arc[256][256];
    for (int i = 0; i < 256; i++) {
	for (int j = 0; j < 256; j++) {
	    arc[i][j] = 0;
	}
    }

    int source = 0, dest = 171, ep1, ep2, bw;
    while (fgets(buf, 256, file)!=NULL) {
	trim(buf);
	printf("source = %d, dest = %d\n", source, dest);

	while(strlen(buf) > 0) {
	    sscanf(buf, "%d %d %d", &ep1, &ep2, &bw);
	    printf("%d %d %d\n", ep1, ep2, bw);
	    arc[ep1][ep2] = bw;
	    fgets(buf, 256, file);
	    trim(buf);
	}

	/*Now we have the matrix, check what connects to what*/
	printPath(arc, source, dest);

	source++;
	dest++;
    }

    return 0;
}
