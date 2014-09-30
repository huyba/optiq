model optiq.mod;
data optiq.dat;
option solver snopt;
solve;

display _ampl_elapsed_time;
display _total_solve_elapsed_time;

param JBS:=card(Jobs)-1;
var k:=0;

for {job in Jobs} {
    for {t in Types} {
	if Demand[job,t] >0 then {
	    printf "Demand: %d %d %s %8.0f\n", Source[job], Destination[job], t, Demand[job,t];
	    printf "   Flows:\n";
	    for {(u,v) in Arcs} {
		if Flow[k,t,u,v] > 0.0005 then
		    printf "%4d %4d %8.0f\n", u, v, Flow[k,t,u,v];
	    }
	}
    }
    let k:=k+1;
    printf "\n";
}
