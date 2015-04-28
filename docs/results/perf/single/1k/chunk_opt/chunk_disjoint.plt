set term postscript
set tmarg 0
set bmarg 0
set lmarg 5
set rmarg 2
set output "chunk_disjoint_1k.ps"
set title "Transfer bandwidth for OPTIQ with different chunk size \n Disjoint pattern, 1024 nodes, 1 rank/node, message size = 8 MB" font ",22"
set logscale y 10
set yrange [4096:2097152]
set xrange [0:15]
set xlabel "Test Id" font ",24"
set ylabel "Total throughput(GB/s)" font ",24" offset -2
set style line 1 lt rgb "blue" lw 7
set style line 2 lt rgb "green" lw 7
set style line 3 lt rgb "skyblue" lw 7
set style line 4 lt rgb "red" lw 7
set style line 5 lt rgb "orange" lw 7
set style line 6 lt rgb "purple" lw 7
set style line 7 lt rgb "pink" lw 7
set style line 8 lt rgb "brown" lw 7
set key font ",24"
set xtics font ",22"
set ytics font ",22"
set key top left spacing 3
set xtics () font ",23"
set ytics ("1" 1024, "4" 4096, "8" 8192, "16" 16384, "32" 32768, "64" 65536, "128" 131072, "256" 262144, "512" 524288, "1024" 1048576) font ",23"
plot "64k.dat" using 1:11 ls 1 title "          Chunk size = 64 KB" with linespoints, \
"128k.dat" using 1:11 ls 2 title "Chunk size = 128 KB" with linespoints, \
"256k.dat" using 1:11 ls 4 title "          Chunk size = 256 KB" with linespoints, \
"512k.dat" using 1:11 ls 6 title "          Chunk size = 512 KB" with linespoints, \
"1m.dat" using 1:11 ls 8 title "          Chunk size = 1 MB" with linespoints, \
"32k.dat" using 1:11 ls 3 title "  Chunk size = 32 KB" with linespoints, \
"16k.dat" using 1:11 ls 5 title "Chunk size = 16 KB" with linespoints
#"8k.dat" using 1:11 ls 7 title "Chunk size = 8 KB" with linespoints, \
#"1m.dat" using 1:11 ls 8 title "          Chunk size = 1 MB" with linespoints
