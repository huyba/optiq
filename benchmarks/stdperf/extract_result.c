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

    int prevtestid = 0, testid, len, chunk;
    char temp[256], temp1[256], temp2[256], temp3[256];
    float mpitime, mpibw, opttime, optbw;
    float mmpitime = DBL_MAX, mmpibw, mopttime = DBL_MAX, moptbw;
    bool newtest = false, newbingo, firsttime = true;
    char s1[256], s2[256], s3[256],s4[256], s5[256],s6[256],s7[256],s8[256],s9[256];
    int mpi_maxload, mpi_minload, optiq_maxload, optiq_minload, mpi_maxhops, mpi_minhops, optiq_maxhops, optiq_minhops;
    float mpi_avgload, optiq_avgload, mpi_avghops, optiq_avghops;

    std::ofstream outfile;
    outfile.open (outpath);

    FILE * fp;
    char line[256];

    fp = fopen(inpath, "r");

    while (fgets(line, 256, fp) != NULL)
    {
	if (line[0] == 'T')
	{
	    trim(line);
            sscanf(line, "%s %s %d", temp, temp1, &testid);

	    if (testid != prevtestid) 
	    {
		outfile << prevtestid << " " << mmpitime << " " << mmpibw << " " << mopttime << " " << moptbw << " " << mpi_maxload << " "  << mpi_minload << " " << mpi_avgload << " " << mpi_maxhops << " " << mpi_minhops << " " << mpi_avghops << " " << optiq_maxload << " " << optiq_minload << " " << optiq_avgload << " " << optiq_maxhops << " " << optiq_minhops << " " << optiq_avghops << std::endl;
		mmpitime = DBL_MAX;
		mopttime = DBL_MAX;
	    }

	    prevtestid = testid;
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

	    fgets(line, 256, fp);

            fgets(line, 256, fp);
            printf("%s\n", line);
            sscanf(line, "%s %s %d", s1, s1, &mpi_maxhops);
            fgets(line, 256, fp);
            printf("%s\n", line);
            sscanf(line, "%s %s %d", s1, s1, &mpi_minhops);
            fgets(line, 256, fp);
            printf("%s\n", line);
            sscanf(line, "%s %s %f", s1, s1, &mpi_avghops);
            fgets(line, 256, fp);
            printf("%s\n", line);
            sscanf(line, "%s %s %d", s1, s1, &mpi_maxload);
            fgets(line, 256, fp);
            printf("%s\n", line);
            sscanf(line, "%s %s %d", s1, s1, &mpi_minload);
            fgets(line, 256, fp);
            printf("%s\n", line);
            sscanf(line, "%s %s %f", s1, s1, &mpi_avgload);
	}

	if (line[0] == 'O')
	{
	    trim(line);
	    sscanf(line, "%s %s %s %d %s %s %s %f, %s %s %f", s1, s2, s3, &len, s4, s5, s6, &opttime, s7, s8, &optbw);
            printf("%s\n", line);
	    if (opttime < mopttime) 
	    {
		moptbw = optbw;
		mopttime = opttime;
	    }

            fgets(line, 256, fp);
            if (line[0] = 'B') {
                fgets(line, 256, fp);
            }
            
	    fgets(line, 256, fp);
            printf("%s\n", line);
	    sscanf(line, "%s %s %d", s1, s1, &optiq_maxhops);
	    fgets(line, 256, fp);
            printf("%s\n", line);
	    sscanf(line, "%s %s %d", s1, s1, &optiq_minhops);
	    fgets(line, 256, fp);
            printf("%s\n", line);
	    sscanf(line, "%s %s %f", s1, s1, &optiq_avghops);
	    fgets(line, 256, fp);
            printf("%s\n", line);
	    sscanf(line, "%s %s %d", s1, s1, &optiq_maxload);
	    fgets(line, 256, fp);
            printf("%s\n", line);
	    sscanf(line, "%s %s %d", s1, s1, &optiq_minload);
	    fgets(line, 256, fp);
            printf("%s\n", line);
	    sscanf(line, "%s %s %f", s1, s1, &optiq_avgload);
	}
    }

    outfile << testid << " " << mmpitime << " " << mmpibw << " " << mopttime << " " << moptbw << " " << mpi_maxload << " "  << mpi_minload << " " << mpi_avgload << " " << mpi_maxhops << " " << mpi_minhops << " " << mpi_avghops << " " << optiq_maxload << " " << optiq_minload << " " << optiq_avgload << " " << optiq_maxhops << " " << optiq_minhops << " " << optiq_avghops  << std::endl;

    outfile.close();
    fclose(fp);

    return 0;
}
