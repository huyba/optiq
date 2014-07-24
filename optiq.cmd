model optiq.mod;
data optiq.dat;
option solver snopt;
solve;
display _ampl_elapsed_time;
display _total_solve_elapsed_time;
#display Demand;
#display Flow;
