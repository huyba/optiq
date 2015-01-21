model model.mod;
data 256to4_16.dat;
option solver snopt;
solve;

display _ampl_elapsed_time;
display _total_solve_elapsed_time;

var k:=0;

for {job in Jobs} {
    for {p in Paths[job]} {
	printf "Job_Paths_Flow %d %d %8.4f\n", job, p, Flow[job, p];
    }
    printf("\n");
}
