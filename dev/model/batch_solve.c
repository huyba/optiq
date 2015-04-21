#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

void gen_cmd_file(char *datfile, char *outputfile, int i)
{
    std::ofstream myfile;
    char filename[256];
    sprintf(filename, "model%d.cmd", i);

    myfile.open (filename);

    myfile << "model /homes/huybui/optiq/dev/model/path_based/model.mod;" << std::endl;
    
    myfile << "data " << datfile << std::endl;

    myfile << "option solver snopt;" << std::endl;
    myfile << "solve;" << std::endl;

    myfile << "display _ampl_elapsed_time;" << std::endl;
    myfile << "display _total_solve_elapsed_time;"  << std::endl;

    myfile << "for {job in Jobs} {" << std::endl;
    myfile << "    for {p in Paths[job]} {"  << std::endl;
    myfile << "        if Flow[job, p] > 0.01 then {" << std::endl;
    myfile << "            printf \"Job_Paths_Flow %d %d %8.4f 0 0 0 0\\n\", job, p, Flow[job, p];" << std::endl;
    myfile << "            for {(u,v) in Path_Arcs[job,p]}{" << std::endl;
    myfile << "                printf \"%d %d\\n\", u, v;" << std::endl;
    myfile << "            }" << std::endl;
    myfile << "            printf \"\\n\";" << std::endl;
    myfile << "        }" << std::endl;
    myfile << "    }" << std::endl;
    myfile << "    printf(\"\\n\");" << std::endl;
    myfile << "}" << std::endl;

    myfile.close();

    char cmd[256];
    sprintf(cmd, "ampl model%d.cmd > %s", i, outputfile);

    system(cmd);

}
int main(int argc, char **argv)
{
    char infile[256];
    char outfile[256];

    char *inbasepath = argv[1];
    char *outbasepath = argv[2];

    int start = 0;
    int end = 90;

    if (argc > 3) {
	start = atoi (argv[3]);
    }

    if (argc > 4) {
	end = atoi (argv[4]);
    }

    for (int i = start; i <= end; i++)
    {
	sprintf(infile, "%s/model%d.dat", inbasepath, i) ;
	sprintf(outfile, "%s/test%d", outbasepath, i);
    
	gen_cmd_file(infile, outfile, i);
    }

    return 0;
}
