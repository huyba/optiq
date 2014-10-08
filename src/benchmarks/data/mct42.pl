set terminal x11

set xlabel "x"
set ylabel "y"
set zlabel "z"

set grid ztics
set grid ytics
set grid xtics

set xrange [0:16]
set yrange [0:7]
set zrange [0:23]

splot 'atm42' using 1:2:3 lt rgb "red" pt 7 ps 1 with points, \
'ocn42' using 1:2:3 lt rgb "blue" pt 7 ps 1 with points, \
'cpl42' using 1:2:3 lt rgb "green" pt 7 ps 1 with points

#set xrange [0:15]
#set yrange [0:5]
#set zrange [0:24]

#splot 'edison' using 1:2:3 pt 7 ps 1 with points
