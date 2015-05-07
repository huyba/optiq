set term postscript
set tmarg 0
set bmarg 0
set lmarg 5
set rmarg 2
set output "time.ps"
set title "Total elapsed time for Optimization in 3 patterns 2K with different max paths" font ",22"
set logscale y 10
set yrange [1:10000]
set xrange []
set xlabel "Test Id" font ",24"
set ylabel "Total time (seconds)" font ",24"
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
set key top right spacing 3
set xtics () font ",23"
set ytics () font ",23"
plot "time4.dat" using 1:4 ls 1 title "             4 Paths" with linespoints, \
"time8.dat" using 1:4 ls 2 title "             8 Paths" with linespoints, \
"time16.dat" using 1:4 ls 4 title "             16 Paths" with linespoints, \
"time32.dat" using 1:4 ls 5 title "             32 Paths" with linespoints, \
"time50.dat" using 1:4 ls 6 title "             50 Paths" with linespoints
