set term postscript
set tmarg 0
set bmarg 0
set lmarg 4
set rmarg 2
set output "opt_paths_disjoint.ps"
set title "Transfer bandwidth for Optimization, 1K nodes with different max paths \n Disjoint" font ",22"
set logscale y 10
set yrange [2048:1048576]
set xrange [0:15]
set xlabel "Test Id" font ",24"
set ylabel "Total throughput(GB/s)" font ",24" offset -1
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
plot "opt_mp4_64k.dat" using 2:12 ls 1 title "             OPTIQ Optimization Max Path = 4" with linespoints, \
"opt_mp8_64k.dat" using 2:12 ls 2 title "     OPTIQ Optimization Max Path = 8" with linespoints, \
"opt_mp16_64k.dat" using 2:12 ls 4 title "     OPTIQ Optimization Max Path = 16" with linespoints, \
"opt_mp32_64k.dat" using 2:12 ls 6 title "     OPTIQ Optimization Max Path = 32" with linespoints, \
"opt_mp50_64k.dat" using 2:12 ls 7 title "     OPTIQ Optimization Max Path = 50" with linespoints
