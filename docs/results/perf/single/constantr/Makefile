all:

collect:
	. collect.sh 3
	. collect.sh 27
	. collect.sh 87
plot:
	gnuplot constantr.plt
	gnuplot constantr_disjoint.plt
	gnuplot constantr_overlap.plt
	gnuplot constantr_subset.plt
pp:
	gnuplot constantr_disjoint_paper.plt
	gnuplot constantr_overlap_paper.plt
	gnuplot constantr_subset_paper.plt
