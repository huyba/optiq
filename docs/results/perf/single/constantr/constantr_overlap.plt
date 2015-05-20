set term postscript
set tmarg 0
set bmarg 0
set lmarg 4
set rmarg 2
set output "constantr_overlap.ps"
set title "Transfer bandwidth from the first N/16 nodes to the last N/2 nodes" font ",22"
set logscale y 10
set logscale x 2
set yrange [4096:1048576]
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
set key font ",22"
set xtics font ",22"
set ytics font ",22"
set key bottom right spacing 3
set xtics ("512" 512, "1024" 1024, "2048" 2048, "4096" 4096, "8192" 8192) font ",23"
set ytics ("16" 16384, "32" 32768, "64" 65536, "128" 131072, "256" 262144, "512" 524288, "1024" 1048576) font ",23"
plot \
"test27_opt.dat" using 1:4 ls 4 title "          Overlap - OPTIQ Optimization" with linespoints, \
"test27_heu_ml1.dat" using 1:4 ls 1 title "     Overlap - OPTIQ Heuristics Maxload = 1" with linespoints, \
"test27_heu_ml2.dat" using 1:4 ls 3 title "     Overlap - OPTIQ Heuristics Maxload = 2" with linespoints, \
"test27_heu_ml4.dat" using 1:4 ls 5 title "     Overlap - OPTIQ Heuristics Maxload = 4" with linespoints, \
"test27_heu_ml8.dat" using 1:4 ls 6 title "     Overlap - OPTIQ Heuristics Maxload = 8" with linespoints, \
"test27_heu_ml16.dat" using 1:4 ls 7 title "     Overlap - OPTIQ Heuristics Maxload = 16" with linespoints, \
"test27_heu_ml32.dat" using 1:4 ls 8 title "     Overlap - OPTIQ Heuristics Maxload = 32" with linespoints, \
"test27_mpi.dat" using 1:4 ls 2 title "     Overlap - MPI_Alltoallv" with linespoints

