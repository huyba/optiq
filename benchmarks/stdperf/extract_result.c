#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>

#include "util.h"

int main (int argc, char **argv)
{
    char *inpath = argv[1];
    char *outpath = argv[2];

    int testid, len, chunk;
    char temp[256], temp1[256], temp2[256], temp3[256];
    float mpitime, mpibw, opttime, optbw;
    float mmpitime = DBL_MAX, mmpibw, mopttime = DBL_MAX, moptbw;
    bool newtest = false, newbingo, firsttime = true;
    char s1[256], s2[256], s3[256],s4[256], s5[256],s6[256],s7[256],s8[256],s9[256];

    std::ofstream outfile;
    outfile.open (outpath);

    FILE * fp;
    char line[256];

    fp = fopen(inpath, "r");

    while (fgets(line, 256, fp) != NULL)
    {
	if (line[0] == 'T')
	{
	    if (!firsttime) 
	    {
		outfile << testid << " " << mmpitime << " " << mmpibw << " " << mopttime << " " << moptbw << std::endl;
		mmpitime = DBL_MAX;
		mopttime = DBL_MAX;
	    } 
	    else 
	    {
		firsttime = false;
	    }

	    trim(line);
	    sscanf(line, "%s %s %d", temp, temp1, &testid);
	}

	if (line[0] == 'M')
	{
	    trim(line);
	    printf("%s\n", line);

	    sscanf(line, "%s %s %s %d %s %s %s %f, %s %s %f", s1, s2, s3, &len, s4, s5, s6, &mpitime, s7, s8, &mpibw);

	    if (mpitime < mmpitime) 
	    {
		mmpibw = mpibw;
		mmpitime = mpitime;
	    }
	}

	if (line[0] == 'O')
	{
	    trim(line);
	    sscanf(line, "%s %s %s %d %s %s %s %f, %s %s %f", s1, s2, s3, &len, s4, s5, s6, &opttime, s7, s8, &optbw);

	    if (opttime < mopttime) 
	    {
		moptbw = optbw;
		mopttime = opttime;
	    }
	}
    }

    outfile << testid << " " << mmpitime << " " << mmpibw << " " << mopttime << " " << moptbw << std::endl;

    outfile.close();
    fclose(fp);

    return 0;
}
