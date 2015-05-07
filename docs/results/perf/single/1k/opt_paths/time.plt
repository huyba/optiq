set term postscript
set tmarg 0
set bmarg 0
set lmarg 5
set rmarg 2
set output "time.ps"
set title "Generating time for Heuristics in 3 patterns 1K with different max load" font ",22"
#set logscale y 10
set yrange [0:60000000]
set xrange []
set xlabel "Max load (number of paths/physical link)" font ",24"
set ylabel "Generating time (seconds)" font ",24" offset -2
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
set ytics ("4" 4000000, "10" 10000000, "20" 20000000, "30" 30000000, "40" 40000000, "50" 50000000, "60" 60000000) font ",23"
plot "time.dat" using 1:2 ls 1 title "           OPTIQ Heuristics Generate Time" with linespoints
