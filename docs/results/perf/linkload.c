#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string.h>

int main (int argc, char **argv)
{
    char *filepath = argv[1];
    char *outfile = argv[2];
    char *mo = argv[3];
    char *pd = argv[4];
    int testid = atoi (argv[5]);
    int chunksize = atoi (argv[6]);

    FILE * fp;
    char line[256];

    if( access( filepath, F_OK ) == -1 ) {
        return false;
    }

    fp = fopen(filepath, "r");

    if (fp == NULL) {
        return false;
    }

    std::ofstream myfile;

    myfile.open (outfile);

    for (int i = 0; i < 10; i++)
    {
	fgets(line, 256, fp);
    }

    int freq = 0, val = 0, msg = 0, chunk = 0, id = 0;
    char s1[256], s2[256], s3[256], s4[256];

    while (fgets(line, 80, fp) != NULL)
    {
        if (line[0] == mo[0])
        {
	    sscanf(line, "%s %d %s %s %s %d %s %s %d", s1, &id, s1, s1, s1, &msg, s1, s1, &chunk);

	    if (testid == id && chunk == chunksize) 
	    {
		while (line[0] != pd[0])
		{
		    fgets(line, 80, fp);
		}

		while (line[0] == pd[0])
		{
		    sscanf(line, "%s %d %s %s %d", s1, &freq, s2, s3, &val);
		    myfile << val << " " << freq << std::endl;
		    fgets(line, 80, fp);
		}
	    }
	}
    }

    fclose(fp);
    myfile.close();
}
