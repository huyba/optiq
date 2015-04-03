set term postscript
set tmarg 0
set bmarg 0
set lmarg 2
set rmarg 2
set output "load.ps"
set title "Aggregation networking load for MPI_Alltoallv and OPTIQ in subgroup" font ",22"
set logscale x 2
set yrange [0:35]
set xrange [3.8:68]
set xlabel "Subgroup size" font ",24"
set ylabel "Load (num. paths per a physical link)" font ",24"
set style line 1 lt rgb "blue" lw 7
set style line 2 lt rgb "green" lw 7
set style line 3 lt rgb "skyblue" lw 7
set style line 4 lt rgb "purple" lw 7
set style line 5 lt rgb "orange" lw 7
set style line 6 lt rgb "brown" lw 7
set style line 7 lt rgb "red" lw 7
set style line 8 lt rgb "black" lw 7
set key font ",24"
set xtics font ",22"
set ytics font ",22"
set key top left spacing 3
set xtics ("4" 4, "8" 8, "16" 16, "32" 32, "64" 64) font ",23"
set ytics () font ",23"
plot "stat.dat" using 1:2 ls 1 title "           Max load MPI_Alltoallv" with linespoints, \
"stat.dat" using 1:4 ls 2 title "Mac load OPTIQ heuristic" with linespoints, \
"stat.dat" using 1:3 ls 3 title "Avg load MPI_Alltoallv" with linespoints, \
"stat.dat" using 1:5 ls 4 title "Avg load OPTIQ heuristic" with linespoints
