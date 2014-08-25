set term postscript
set tmarg 0
set bmarg 0
set lmarg 6
set rmarg 4
set output "g2g_5D.ps"
set title "Solving time to find minimum time to transfer data from sources to dests" font ",22"
set logscale x 2
set logscale y 10
set xrange [32:512]
set yrange [1:1000]
set xlabel "Total number of nodes" font ",24"
set ylabel "Solving time(ms)" font ",24"
set style line 1 lt rgb "red" lw 7
set style line 2 lt rgb "orange" lw 7
set style line 3 lt rgb "purple" lw 7
set style line 4 lt rgb "blue" lw 7
set style line 5 lt rgb "green" lw 7
set style line 6 lt rgb "skyblue" lw 7
set style line 7 lt rgb "brown" lw 7
set style line 8 lt rgb "yellow" lw 7
set style line 9 lt rgb "violet" lw 7
set style line 10 lt rgb "cyan" lw 7
set style line 11 lt rgb "magenta" lw 7
set style line 12 lt rgb "navy" lw 7
set key font ",24"
set xtics font ",22"
set ytics font ",22"
set key bottom right spacing 3
set xtics font ",23"
set ytics font ",23"
plot "g2g_5D" using 2:6 ls 1 title "1/8 nodes as sources and 1/8 nodes as dests" with linespoints