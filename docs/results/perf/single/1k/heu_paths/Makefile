all:
dat:
	. collect.sh 1
	. collect.sh 2
	. collect.sh 4
	. collect.sh 8
	. collect.sh 16
	. collect.sh 32

plot:
	gnuplot heu_paths_3p.plt
	gnuplot disjoint.plt
	gnuplot overlap.plt
	gnuplot subset.plt
	gnuplot allpatterns.plt
