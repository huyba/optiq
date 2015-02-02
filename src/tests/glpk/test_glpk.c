#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <mpi.h>

#include <glpk.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

     struct timeval t1, t2;
    long int diff = 0L;

    char *model_file = argv[1];
    char *data_file = argv[2];

    glp_prob *prob;
    glp_tran *tran;
    int ret;
    prob = glp_create_prob();
    tran = glp_mpl_alloc_wksp();

    gettimeofday(&t1, NULL);

    ret = glp_mpl_read_model(tran, model_file, 1);

    gettimeofday(&t2, NULL);

    diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);

    printf("Time to read model is %ld (microsecond)\n", diff);

    if (ret != 0)
    {
        fprintf(stderr, "Error on translating model\n");
    }

    gettimeofday(&t1, NULL);

    ret = glp_mpl_read_data(tran, data_file);
    if (ret != 0)
    {
        fprintf(stderr, "Error on translating data\n");
    }

    gettimeofday(&t2, NULL);

    diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);

    printf("Time to read data is %ld (microsecond)\n", diff);

    gettimeofday(&t1, NULL);
    
    ret = glp_mpl_generate(tran, NULL);
    if (ret != 0)
    {
        fprintf(stderr, "Error on generating model\n");
    }

    gettimeofday(&t2, NULL);

    diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);

    printf("Time to generate is %ld (microsecond)\n", diff);

    gettimeofday(&t1, NULL);

    glp_mpl_build_prob(tran, prob);

    gettimeofday(&t2, NULL);

    diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);

    printf("Time to build is %ld (microsecond)\n", diff);

    gettimeofday(&t1, NULL);

    glp_simplex(prob, NULL);

    gettimeofday(&t2, NULL);

    diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);

    printf("Time to solve using simplex is %ld (microsecond)\n", diff);

    gettimeofday(&t1, NULL);

    glp_intopt(prob, NULL);

    gettimeofday(&t2, NULL);

    diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);

    printf("Time to solve using intopt is %ld (microsecond)\n", diff);

    gettimeofday(&t1, NULL);

    glp_interior(prob, NULL);

    gettimeofday(&t2, NULL);

    diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);

    printf("Time to solve using interior is %ld (microsecond)\n", diff);

    ret = glp_mpl_postsolve(tran, prob, GLP_SOL);

    if (ret != 0)
    {
       fprintf(stderr, "Error on postsolving model\n");
    }

    glp_mpl_free_wksp(tran);
    glp_delete_prob(prob);

    return 0;

    MPI_Finalize();

    return 0;
}
