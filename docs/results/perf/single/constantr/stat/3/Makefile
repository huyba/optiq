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
	. tableshort.sh 3 512 512
	. tableshort.sh 3 1024 1k
	. tableshort.sh 3 2048 2k
	. tableshort.sh 3 4096 4k
	. tableshort.sh 3 8192 8k

	. tableshort.sh 27 512 512
	. tableshort.sh 27 1024 1k
	. tableshort.sh 27 2048 2k
	. tableshort.sh 27 4096 4k
	. tableshort.sh 27 8192 8k

	. tableshort.sh 87 512 512
	. tableshort.sh 87 1024 1k
	. tableshort.sh 87 2048 2k
	. tableshort.sh 87 4096 4k

csv:
	. csv.sh 512
	. csv.sh 1k
	. csv.sh 2k
	. csv.sh 4k
	. csv.sh 8k
