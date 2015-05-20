set terminal postscript

set tmarg 0
set bmarg 0
set lmarg 3
set rmarg 2

set output "loadpath_histo.ps"
set title "Distribution of number of paths over physical links"  font ",22"

set logscale y 2
set yrange []
set xrange [-1:18]
set xlabel "Number of paths sharing a link" font ",24" 
set ylabel "Number of links" font ",24" offset 1.5

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
set xtics ("0" 0, "1" 1, "2" 2, "3" 3, "4" 4, "5" 5, "6" 6, "7" 7, "8" 8, "9" 9, "10" 10, "12" 12, "14" 14, "16" 16, "18" 18, "20" 20, "22" 22, "24" 24, "28" 28) font ",23"

plot 'opt_loadpath_87_64k.dat' using 2 title "Optimization" ls 1, \
'heu_ml4_loadpath_87_64k.dat' using 2 title "Heuristics Max Load = 4" ls 2, \
'mpi_loadpath_87_64k.dat' using 2 title "MPI_Alltoallv" ls 3

