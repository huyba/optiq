reset
n=100#number of intervals
max=26. #max value
min=16. #min value
width=(max-min)/n/2 #interval width

#function used to map a value to the intervals
hist(x,width)=width*floor(log(x)/log(2.0)/width/3.0)*3.0+width/2.0

set term postscript #output terminal and file
set tmarg 0
set bmarg 0
set lmarg 4
set rmarg 2
set output "loaddata_histo.ps"
set title "Distribution of data sizes over physical links" font ",22"

#set logscale y 2
set xrange [min:max]
set yrange []

#to put an empty boundary around the
#data inside an autoscaled graph.
set offset graph 0.0,0.05,0.05,0.0

set key top left spacing 3
set key font ",22"

set xtics ("64K" 16, "128K" 17, "256K" 18,  "512K" 19, "1M" 20, "2M" 21, "4M" 22, "8M" 23, "16M" 24, "32M" 25, "64M" 26) font ",22"
set ytics () font ",22"

set boxwidth width*1.0

set style histogram clustered gap 2 title textcolor lt -1
set style data histograms

set style line 1 lc rgb "red"
set style line 2 lc rgb "blue"
set style line 3 lc rgb "green"

set style fill solid 1.0 #fillstyle
set tics out nomirror
set xlabel "Data size (Bytes) per Link" font ",23"
set ylabel "Number of Links" font ",23"
#count and plot
plot "opt_loaddata_87_64k.dat" u (hist($1,width)+width):($2) smooth freq w boxes ls 1 title "       Optimization", \
"heu_loaddata_87_64k.dat" u (hist($1,width)):($2) smooth freq w boxes ls 2 title "       Heuristics", \
"mpi_loaddata_87.dat" u (hist($1,width)+width*2.0):($2) smooth freq w boxes ls 3 title "MPI_Alltoallv"
