model optiq.mod;
data optiq_5.dat;
option solver snopt;
solve;

display _ampl_elapsed_time;
display _total_solve_elapsed_time;

param JBS:=card(Jobs)-1;
var k:=0;
var c:=0;

for {job in Jobs} {
    if Demand[job] >0 then {
	printf "Demand: %d %d %s %8.0f\n", Source[job], Destination[job], Class[job], Demand[job];
	printf "   Flows:\n";
	let c:=Class[job];
	for {(u,v) in Arcs} {
	    if Flow[k,c,u,v] > 0.0005 then
		printf "%4d %4d %8.0f\n", u, v, Flow[k,c,u,v];
	    }
	}
    let k:=k+1;
    printf "\n";
}
