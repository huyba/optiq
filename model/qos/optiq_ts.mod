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

var Flow {j in Jobs, t in Types, Arcs} >= 0, <= Demand[j, t];
var Z {Types} >= 0;

var total_flow {(i,j) in Arcs} = sum {job in Jobs, t in Types} Flow[job, t, i, j];
maximize obj: sum {t in Types} Weight[t]*Z[t];

subject to

zero_flow {job in Jobs, t in Types, i in Nodes}:

sum {(i,j) in Arcs} Flow[job, t,i,j] - sum {(j,i) in Arcs}   Flow[job, t,j,i] =

if (i == Source[job]) then Demand[job, t]*Z[t] else if (i == Destination[job]) then -Demand[job, t]*Z[t]   else 0;
sumZ: sum {t in Types} Z[t] = 1;
capacity {(i,j) in Arcs}: total_flow[i,j] <= Capacity[i,j];

