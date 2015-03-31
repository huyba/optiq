model /home/abui/workspace/optiq/dev/model/path_based/model.mod;
data /gpfs/mira-fs0/projects/Performance/abui/optiq/tests/optiq/pathbased/model.dat;
option solver snopt;
solve;

display _ampl_elapsed_time;
display _total_solve_elapsed_time;

for {job in Jobs} {
    for {p in Paths[job]} {
	if Flow[job, p] > 0.01 then {
	    printf "Job_Paths_Flow %d %d %8.4f\n", job, p, Flow[job, p];
	    for {(u,v) in Path_Arcs[job,p]} {
		printf "%d %d\n", u, v;
	    }
	    printf "\n";
	}
    }
    printf("\n");
}
