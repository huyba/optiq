#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include <glpk.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    glp_prob *mip;
    glp_tran *tran;
    int ret;
    mip = glp_create_prob();
    tran = glp_mpl_alloc_wksp();
    ret = glp_mpl_read_model(tran, "model.mod", 1);

    if (ret != 0)
    {
        fprintf(stderr, "Error on translating model\n");
    }

    ret = glp_mpl_read_data(tran, "512to8_3.dat");
    if (ret != 0)
    {
        fprintf(stderr, "Error on translating data\n");
    }

    ret = glp_mpl_generate(tran, NULL);
    if (ret != 0)
    {
        fprintf(stderr, "Error on generating model\n");
    }

    glp_mpl_build_prob(tran, mip);
    glp_simplex(mip, NULL);
    glp_intopt(mip, NULL);

    ret = glp_mpl_postsolve(tran, mip, GLP_MIP);

    if (ret != 0)
    {
       fprintf(stderr, "Error on postsolving model\n");
    }

    glp_mpl_free_wksp(tran);
    glp_delete_prob(mip);

    return 0;

    MPI_Finalize();

    return 0;
}
