model optiq.mod;
data optiq_s.dat;
option solver snopt;
solve;

display _ampl_elapsed_time;
display _total_solve_elapsed_time;

param JBS:=card(Jobs)-1;
var k:=0;

for {job in Jobs} {
    printf "J %d %d %d %8.0f \n", k, Source[job], Destination[job], Demand[job];
    for {(u,v) in Arcs} {
	if Flow[k,u,v] > 0.05 then
	    printf "%4d %4d %8.3f\n", u, v, Flow[k,u,v];
    }
    let k:=k+1;
    printf "\n";
}

