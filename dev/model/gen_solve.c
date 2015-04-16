#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

void gen_cmd_file(char *datfile, char *outputfile)
{
    std::ofstream myfile;
    myfile.open ("model.cmd");

    myfile << "model /homes/huybui/optiq/dev/model/path_based/model.mod;" << std::endl;
    
    myfile << "data " << datfile << std::endl;

    myfile << "option solver snopt;" << std::endl;
    myfile << "solve;" << std::endl;

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

    char cmd[256];
    sprintf(cmd, "ampl model.cmd > %s", outputfile);

    system(cmd);

}
int main(int argc, char **argv)
{
    gen_cmd_file("./path_based/data/model0.dat", "flows/flow0.dat");
}
