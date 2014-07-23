model optiq.mod;
data optiq_5D_256.dat;
option solver snopt;
solve;
display _ampl_elapsed_time;
#display Demand;
#display Flow;
