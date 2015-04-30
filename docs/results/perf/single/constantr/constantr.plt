set term postscript
set tmarg 0
set bmarg 0
set lmarg 4
set rmarg 2
set output "constantr.ps"
set title "Transfer bandwidth from the first N/16 nodes to the last N/2 nodes" font ",22"
set logscale y 10
set logscale x 2
set yrange [4096:262144]
set xlabel "Partition size (N)" font ",24"
set ylabel "Total throughput(GB/s)" font ",24" offset -2
set style line 1 lt rgb "blue" lw 7
set style line 2 lt rgb "green" lw 7
set style line 3 lt rgb "skyblue" lw 7
set style line 4 lt rgb "red" lw 7
set style line 5 lt rgb "orange" lw 7
set style line 6 lt rgb "pink" lw 7
set style line 7 lt rgb "brown" lw 7
set style line 8 lt rgb "purple" lw 7
set style line 9 lt rgb "cyan" lw 7
set key font ",24"
set xtics font ",22"
set ytics font ",22"
set key bottom left spacing 3
set xtics ("512" 512, "1024" 1024, "2048" 2048, "4096" 4096, "8192" 8192) font ",23"
set ytics ("16" 16384, "32" 32768, "64" 65536, "128" 131072, "256" 262144, "512" 524288, "1024" 1048576) font ",23"
plot \
"test87_opt.dat" using 1:4 ls 1 title "          Subset - OPTIQ Optimization" with linespoints, \
"test87_heu.dat" using 1:4 ls 2 title "     Subset - OPTIQ Heuristics" with linespoints, \
"test87_mpi.dat" using 1:4 ls 3 title "     Subset - MPI_Alltoallv" with linespoints, \
"test3_opt.dat" using 1:4 ls 4 title "          Disjoint - OPTIQ Optimization" with linespoints, \
"test3_heu.dat" using 1:4 ls 5 title "     Disjoint - OPTIQ Heuristics" with linespoints, \
"test3_mpi.dat" using 1:4 ls 6 title "     Disjoint - MPI_Alltoallv" with linespoints, \
"test27_opt.dat" using 1:4 ls 7 title "          Overlap - OPTIQ Optimization" with linespoints, \
"test27_heu.dat" using 1:4 ls 8 title "     Overlap - OPTIQ Heuristics" with linespoints, \
"test27_mpi.dat" using 1:4 ls 9 title "     Overlap - MPI_Alltoallv" with linespoints

