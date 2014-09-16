set Nodes;
set Arcs within Nodes cross Nodes;
set Jobs;
set Types;

param Delay {Arcs} default 0;
param Capacity {Arcs} >= 0 default Infinity;
param Source {Jobs};
param Destination {Jobs} default 0;
param Demand {Jobs, Types} default 0;
param Weight {Types} default 1, >= 0;

var Flow {j in Jobs, Types, Arcs} >= 0, <= Demand[j];
var Z{Types} >= 0;

var total_flow{(i,j) in Arcs} = sum {job in Jobs, type in Types} Flow[job, type,i,j];
total_flow[i,j] <= Capacity[i,j];
maximize obj: sum{type in Types} Weight[type]*Z[type];

subject to

zero_flow {job in Jobs, type in Types, i in Nodes}:

sum{(i,j) in Arcs} Flow[job, type,i,j] - sum{(j,i) in Arcs}   Flow[job, type,j,i] =

if (i == Source[job]) then Demand[job, type]*Z[type] else if (i == Destination[job]) then -Demand[job, type]*Z[type]   else 0;
sumZ: sum{type in Types} Z[type] = 1;
capacity {(i,j) in Arcs}: total_flow[i,j] <= Capacity[i,j];

