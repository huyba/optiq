model optiq.mod;
data optiq85.dat;
option solver snopt;
solve;

display _ampl_elapsed_time;
display _total_solve_elapsed_time;

param JBS:=card(Jobs)-1;

for {i in 0..JBS} {
    for {(u,v) in Arcs} {
	if Flow[i,u,v] > 0.0005 then
	    printf "%4d %4d %8.0f\n", u, v, Flow[i,u,v];
    }
    printf "\n";
}

