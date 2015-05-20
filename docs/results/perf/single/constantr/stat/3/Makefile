all: dat64k plot64k datfig plothisto dattab

dat64k:
	. extract64k.sh
plot64k:
	gnuplot disjoint.plt
	gnuplot subset.plt

datfig:
	. extract.sh

plothisto:
	gnuplot hop_histo.plt
	gnuplot copy_histo.plt
	gnuplot loaddata_histo.plt
	gnuplot loadpath_histo.plt
	gnuplot hopbyte_histo.plt
	gnuplot hopcopy_histo.plt

table:
	. table1.sh 3 512 512
	. table1.sh 3 1024 1k
	. table2.sh 3 2048 2k
	. table2.sh 3 4096 4k
	. table3.sh 3 8192 8k

	. table1.sh 27 512 512
	. table1.sh 27 1024 1k
	. table2.sh 27 2048 2k
	. table2.sh 27 4096 4k
	. table3.sh 27 8192 8k

	. table1.sh 87 512 512
	. table1.sh 87 1024 1k
	. table2.sh 87 2048 2k
	. table2.sh 87 4096 4k

csv:
	. csv.sh 512
	. csv.sh 1k
	. csv.sh 2k
	. csv.sh 4k
	. csv.sh 8k
