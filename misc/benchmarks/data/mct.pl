set terminal x11

set xlabel "x"
set ylabel "y"
set zlabel "z"

set xrange [0:16]
set yrange [0:17]
set zrange [0:23]
splot 'atm32' using 1:2:3 lt rgb "red" pt 7 ps 1 with points, \
'ocn32' using 1:2:3 lt rgb "blue" pt 7 ps 1 with points, \
'cpl32' using 1:2:3 lt rgb "violet" pt 7 ps 1 with points

#set xrange [0:15]
#set yrange [0:5]
#set zrange [0:24]

#splot 'edison' using 1:2:3 pt 7 ps 1 with points
