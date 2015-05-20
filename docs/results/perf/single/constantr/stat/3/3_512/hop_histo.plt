set terminal postscript

set tmarg 0
set bmarg 0
set lmarg 4
set rmarg 2

set output "hop_histo.ps"
set title "Distribution of number of hops for paths"  font ",22"

set logscale y 2
set yrange []
set xrange []
set xlabel "Number of hops" font ",24" 
set ylabel "Number of paths" font ",24" 

set boxwidth 0.9 absolute
set style fill   solid 1.00 border lt -1
set key inside right top vertical Right noreverse noenhanced autotitle nobox
set style histogram clustered gap 2 title textcolor lt -1
set datafile missing '-'
set style data histograms

set style line 1 lc rgb "red"
set style line 2 lc rgb "blue"
set style line 3 lc rgb "green"

set key font ",24"
set xtics font ",22"
set ytics font ",22"
set key top right spacing 3
set xtics () font ",23"

plot 'opt_hops_3_64k.dat' using 2 title "Optimization" ls 1, \
'heu_ml4_hops_3_64k.dat' using 2:xtic(1) title "Heuristcs Max Load = 4" ls 2, \
'mpi_hops_3_64k.dat' using 2 title "MPI_Alltoallv" ls 3

