#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    char *path = argv[1];
    char cmd[1024];

    for (int i = 0; i <= 90; i++)
    {
	sprintf(cmd, "cat %s/test%d_* > test%d", path, i, i);
	printf("%s\n", cmd);
	system(cmd);
	sprintf(cmd, "rm %s/test%d_*", path, i);
	printf("%s\n", cmd);
	system(cmd);
    }

    return 0;
}
