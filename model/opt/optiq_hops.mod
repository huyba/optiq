set Nodes;
set Arcs within Nodes cross Nodes;
set Jobs;

param Delay {Arcs} default 0;
param Hop {Arcs} >= 0 default 1;
param Capacity {Arcs} >= 0 default Infinity;
param Source {Jobs};
param Destination {Jobs} default 0;
param Demand {Jobs} default 0;
param MaxHop {Jobs} default 9;

var Flow {Jobs, Arcs} >= 0;
var Z >= 0;

var total_flow{(i,j) in Arcs} = sum {job in Jobs} Flow[job,i,j];
var total_hops{job in Jobs} = sum {(i,j) in Arcs} Hop[i,j] * (if (Flow[job,i,j] > 0.5) then 1 else 0);

maximize obj: Z;

subject to

zero_flow {job in Jobs, i in Nodes}:
sum{(i,j) in Arcs} Flow[job,i,j] - sum{(j,i) in Arcs} Flow[job,j,i] =
if (i == Source[job]) then Demand[job]*Z else if (i == Destination[job]) then -Demand[job]*Z else 0;

capacity {(i,j) in Arcs}:
total_flow[i,j] <= Capacity[i,j];

longest_hops {job in Jobs}:
total_hops[job] <= MaxHop[job];
