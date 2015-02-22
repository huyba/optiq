set Nodes;
set OD within Nodes cross Nodes;    # Origin/Destination pairs
set Arcs within Nodes cross Nodes;
set Paths{OD};
set Path_Arcs{od in OD, p in Paths[od]} within Arcs;

param Demand{OD} default 0;
param Capacity {Arcs} >= 0 default Infinity;

var Flow {Paths} >= 0;
var Z >= 0;

var total_flow{(i,j) in Arcs} = sum {path in Paths} Flow[path];

maximize obj: Z;

subject to

zero_flow {od in OD, i in Nodes}:
sum{(i,j) in Arcs} Flow[od,i,j] - sum{(j,i) in Arcs} Flow[job,j,i] =
if (i == Source[job]) then Demand[job]*Z else if (i == Destination[job]) then -Demand[job]*Z else 0;

capacity {(i,j) in Arcs}:
total_flow[i,j] <= Capacity[i,j];

