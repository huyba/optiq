set Nodes;
set Arcs within Nodes cross Nodes;
set JobID;

param k;
param Delay {Arcs} default 0;
param Capacity {Arcs} >= 0 default Infinity;
param Source {JobID};
param Destination {JobID} default 0;
param Demand {JobID} default 0;

var Flow {JobID, Arcs} >= 0;
var Z >= 0;

var total_flow{(i,j) in Arcs} = sum {job in JobID} Flow[job,i,j];

maximize obj: Z;

subject to

zero_flow {job in JobID, i in Nodes}:
sum{(i,j) in Arcs} Flow[job,i,j] - sum{(j,i) in Arcs} Flow[job,j,i] =
if (i == Source[job]) then Demand[job]*Z else if (i == Destination[job]) then -Demand[job]*Z else 0;

capacity {(i,j) in Arcs}:
total_flow[i,j] <= Capacity[i,j];
