model optiq.mod;
data optiq.dat;
option solver snopt;
solve;
display Demand;
display Flow;
