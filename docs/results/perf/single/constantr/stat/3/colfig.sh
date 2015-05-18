#!/bin/sh

id=$1
opt=../../../$2/data/opt_mp50.dat
heu=../../../$2/data/heu_ml4.dat

./linkload.x $opt opt_hops_$id\_64k.dat O H $id 65536
./linkload.x $opt opt_copies_$id\_64k.dat O C $id 65536
./linkload.x $opt opt_loaddata_$id\_64k.dat O D $id 65536
./linkload.x $opt opt_loadpath_$id\_64k.dat O P $id 65536
./linkload.x $opt opt_hopbyte_$id\_64k.dat O B $id 65536
./linkload.x $opt opt_hopcopy_$id\_64k.dat O Y $id 65536

./linkload.x $opt mpi_hops_$id\_64k.dat M H $id 65536
./linkload.x $opt mpi_copies_$id\_64k.dat M C $id 65536
./linkload.x $opt mpi_loaddata_$id\_64k.dat M D $id 65536
./linkload.x $opt mpi_loadpath_$id\_64k.dat M P $id 65536
./linkload.x $opt mpi_hopbyte_$id\_64k.dat M B $id 65536
./linkload.x $opt mpi_hopcopy_$id\_64k.dat M Y $id 65536

./linkload.x $heu heu_ml4_hops_$id\_64k.dat O H $id 65536
./linkload.x $heu heu_ml4_copies_$id\_64k.dat O C $id 65536
./linkload.x $heu heu_ml4_loaddata_$id\_64k.dat O D $id 65536
./linkload.x $heu heu_ml4_loadpath_$id\_64k.dat O P $id 65536
./linkload.x $heu heu_ml4_hopbyte_$id\_64k.dat O B $id 65536
./linkload.x $heu heu_ml4_hopcopy_$id\_64k.dat O Y $id 65536
