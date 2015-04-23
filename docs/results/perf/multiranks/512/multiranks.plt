set term postscript
set tmarg 0
set bmarg 0
set lmarg 4
set rmarg 2
set output "multiranks_512.ps"
set title "Total networking throughput of OPTIQ vs. MPI_Alltoallv \n 512 nodes, 8 ranks/node, message size = 8 MB" font ",22"
set logscale y 10
set yrange [64:524288]
set xrange [0:90]
set xlabel "Test Id" font ",24" offset -1
set ylabel "Total throughput(GB/s)" font ",24" offset -2
set style line 1 lt rgb "blue" lw 7
set style line 2 lt rgb "green" lw 7
set style line 3 lt rgb "skyblue" lw 7
set style line 4 lt rgb "red" lw 7
set style line 5 lt rgb "orange" lw 7
set style line 6 lt rgb "purple" lw 7
set style line 7 lt rgb "brown" lw 7
set style line 8 lt rgb "black" lw 7
set key font ",24"
set xtics font ",22"
set ytics font ",22"
set key bottom right spacing 3
set xtics () font ",23"
set ytics ("16" 16384, "32" 32768, "64" 65536, "128" 131072, "256" 262144, "512" 524288) font ",23"
plot "c8.dat" using 1:5 ls 1 title "          OPTIQ Optimization Model" with linespoints, \
"c8.dat" using 1:3 ls 6 title "          MPI_Alltoallv" with linespoints
