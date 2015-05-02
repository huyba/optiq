set term postscript
set tmarg 0
set bmarg 0
set lmarg 2
set rmarg 2
set output "allpatterns.ps"
set title "Transfer bandwidth for MPI_Alltoallv and OPTIQ in 3 patterns 1K" font ",22"
set logscale y 10
set yrange [4096:1048576]
set xrange [-1:91]
set xlabel "Test Id" font ",24"
set ylabel "Total throughput(GB/s)" font ",24"
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
set key bottom left spacing 3
set xtics () font ",23"
set ytics ("16" 16384, "32" 32768, "64" 65536, "128" 131072, "256" 262144, "512" 524288, "1024" 1048576) font ",23"
plot "opt64k.dat" using 1:11 ls 1 title "     OPTIQ Optimization Model" with linespoints, \
"old/model30.dat" using 1:5 ls 3 title "     OPTIQ Optimization mxload 30" with linespoints
#"heu64k.dat" using 1:11 ls 2 title "     OPTIQ Heuristics" with linespoints, \
#"mpi64k.dat" using 1:11 ls 6 title "     MPI_Alltoallv" with linespoints


