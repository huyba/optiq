set term postscript
set tmarg 0
set bmarg 0
set lmarg 4
set rmarg 5
set output "distribution.ps"
set title "Load/link distribution for subgroup aggregation using  MPI_Alltoallv and OPTIQ" font ",22"
set logscale y 2
set yrange [1:1024]
set xrange [0:33]
set xlabel "Load" font ",24"
set ylabel "Number of links" font ",24"
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
set xtics ("1" 1, "2" 2, "4" 4, "8" 8, "16" 16, "32" 32) font ",23"
set ytics () font ",23"

set boxwidth 0.5 absolute
set style fill solid 1.00
set style data histogram
set style histogram cluster gap 2
set ylabel "Number of links"
set xlabel "Load"

plot 'distribution.dat' using 2 ls 1 title "            Load/links distribution MPI_Alltoallv" , '' using 3 ls 2 title "            Load/links distribution OPTIQ" 

