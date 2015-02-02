#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
   
    system("/home/abui/lib/glpk/bin/glpsol --model model.mod --data 512to8_3.dat --output output > error");

    MPI_Finalize();

    return 0;    
}
