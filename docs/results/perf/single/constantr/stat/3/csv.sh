#!/bin/sh

nodes=$1
opt=../../../$1/data/opt_mp50.dat
heu1=../../../$1/data/heu_ml1.dat
heu2=../../../$1/data/heu_ml2.dat
heu4=../../../$1/data/heu_ml4.dat
heu8=../../../$1/data/heu_ml8.dat
heu16=../../../$1/data/heu_ml16.dat
heu32=../../../$1/data/heu_ml32.dat

out1=../../../../csv/$1_opt.txt
out2=../../../../csv/$1_heuml1.txt
out3=../../../../csv/$1_heuml2.txt
out4=../../../../csv/$1_heuml4.txt
out5=../../../../csv/$1_heuml8.txt
out6=../../../../csv/$1_heuml16.txt
out7=../../../../csv/$1_heuml32.txt
out8=../../../../csv/$1_mpi.txt

> $out1
>$out2
> $out3
>$out4
> $out5
>$out6
> $out7
> $out8

echo "testid msgsize chunksize datasize time bandwidth numpaths/job maxpath/job minpath/job avgpath/job medpath/job totalhopbytes maxhopbytes minhopbytes avghopbytes medhopbytes totalcopies maxcopies/path mincopies/paths avgcopies/paths medcopies/path totalhops maxhops/path minhop/path avghops/path medhop/path totalcopies/node maxopies/node mincopies/node avgcopies/node medcopies/node maxloadpath minloadpath avgloadpath medloadpath maxloadlink minloadlink avgloadlink medloadlink totalrput maxrputs minrput avgrput medrput" >> $out1

grep "OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $opt | awk '{print $2 " " $6 " " $9 " " $10 " " int($11) " " int($12) " " $13 " " $14 " " $15 " " $16 " " $17 " " $18 " " $19 " " $20 " " $21 " " $22 " " $23 " " $24 " " $25 " " $26 " " $27 " " $28 " " $29 " " $30 " " $31 " " $32 " " $33 " " $34 " " $35 " " $36 " " $37 " " $38 " " $39 " " $40 " " $41 " " $42 " " $43 " " $44 " " $45 " " $46 " " $47 " " $48 " " $49 " " $50}' >> $out1

echo "testid msgsize chunksize datasize time bandwidth numpaths/job maxpath/job minpath/job avgpath/job medpath/job totalhopbytes maxhopbytes minhopbytes avghopbytes medhopbytes totalcopies maxcopies/path mincopies/paths avgcopies/paths medcopies/path totalhops maxhops/path minhop/path avghops/path medhop/path totalcopies/node maxopies/node mincopies/node avgcopies/node medcopies/node maxloadpath minloadpath avgloadpath medloadpath maxloadlink minloadlink avgloadlink medloadlink totalrput maxrputs minrput avgrput medrput" >> $out8

grep "MPI_Alltoallv  msg = 8388608 chunk = 65536" $opt | awk '{print $2 " " $6 " " $9 " " $10 " " int($11) " " int($12) " " $13 " " $14 " " $15 " " $16 " " $17 " " $18 " " $19 " " $20 " " $21 " " $22 " " $23 " " $24 " " $25 " " $26 " " $27 " " $28 " " $29 " " $30 " " $31 " " $32 " " $33 " " $34 " " $35 " " $36 " " $37 " " $38 " " $39 " " $40 " " $41 " " $42 " " $43 " " $44 " " $45 " " $46 " " $47 " " $48 " " $49 " " $50}' >> $out8

echo "testid msgsize chunksize datasize time bandwidth numpaths/job maxpath/job minpath/job avgpath/job medpath/job totalhopbytes maxhopbytes minhopbytes avghopbytes medhopbytes totalcopies maxcopies/path mincopies/paths avgcopies/paths medcopies/path totalhops maxhops/path minhop/path avghops/path medhop/path totalcopies/node maxopies/node mincopies/node avgcopies/node medcopies/node maxloadpath minloadpath avgloadpath medloadpath maxloadlink minloadlink avgloadlink medloadlink totalrput maxrputs minrput avgrput medrput" >> $out2

grep "OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu1 | awk '{print $2 " " $6 " " $9 " " $10 " " int($11) " " int($12) " " $13 " " $14 " " $15 " " $16 " " $17 " " $18 " " $19 " " $20 " " $21 " " $22 " " $23 " " $24 " " $25 " " $26 " " $27 " " $28 " " $29 " " $30 " " $31 " " $32 " " $33 " " $34 " " $35 " " $36 " " $37 " " $38 " " $39 " " $40 " " $41 " " $42 " " $43 " " $44 " " $45 " " $46 " " $47 " " $48 " " $49 " " $50}' >> $out2

echo "testid msgsize chunksize datasize time bandwidth numpaths/job maxpath/job minpath/job avgpath/job medpath/job totalhopbytes maxhopbytes minhopbytes avghopbytes medhopbytes totalcopies maxcopies/path mincopies/paths avgcopies/paths medcopies/path totalhops maxhops/path minhop/path avghops/path medhop/path totalcopies/node maxopies/node mincopies/node avgcopies/node medcopies/node maxloadpath minloadpath avgloadpath medloadpath maxloadlink minloadlink avgloadlink medloadlink totalrput maxrputs minrput avgrput medrput" >> $out3

grep "OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu2 | awk '{print $2 " " $6 " " $9 " " $10 " " int($11) " " int($12) " " $13 " " $14 " " $15 " " $16 " " $17 " " $18 " " $19 " " $20 " " $21 " " $22 " " $23 " " $24 " " $25 " " $26 " " $27 " " $28 " " $29 " " $30 " " $31 " " $32 " " $33 " " $34 " " $35 " " $36 " " $37 " " $38 " " $39 " " $40 " " $41 " " $42 " " $43 " " $44 " " $45 " " $46 " " $47 " " $48 " " $49 " " $50}' >> $out3

echo "testid msgsize chunksize datasize time bandwidth numpaths/job maxpath/job minpath/job avgpath/job medpath/job totalhopbytes maxhopbytes minhopbytes avghopbytes medhopbytes totalcopies maxcopies/path mincopies/paths avgcopies/paths medcopies/path totalhops maxhops/path minhop/path avghops/path medhop/path totalcopies/node maxopies/node mincopies/node avgcopies/node medcopies/node maxloadpath minloadpath avgloadpath medloadpath maxloadlink minloadlink avgloadlink medloadlink totalrput maxrputs minrput avgrput medrput" >> $out4

grep "OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu4 | awk '{print $2 " " $6 " " $9 " " $10 " " int($11) " " int($12) " " $13 " " $14 " " $15 " " $16 " " $17 " " $18 " " $19 " " $20 " " $21 " " $22 " " $23 " " $24 " " $25 " " $26 " " $27 " " $28 " " $29 " " $30 " " $31 " " $32 " " $33 " " $34 " " $35 " " $36 " " $37 " " $38 " " $39 " " $40 " " $41 " " $42 " " $43 " " $44 " " $45 " " $46 " " $47 " " $48 " " $49 " " $50}' >> $out4

echo "testid msgsize chunksize datasize time bandwidth numpaths/job maxpath/job minpath/job avgpath/job medpath/job totalhopbytes maxhopbytes minhopbytes avghopbytes medhopbytes totalcopies maxcopies/path mincopies/paths avgcopies/paths medcopies/path totalhops maxhops/path minhop/path avghops/path medhop/path totalcopies/node maxopies/node mincopies/node avgcopies/node medcopies/node maxloadpath minloadpath avgloadpath medloadpath maxloadlink minloadlink avgloadlink medloadlink totalrput maxrputs minrput avgrput medrput" >> $out5

grep "OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu8 | awk '{print $2 " " $6 " " $9 " " $10 " " int($11) " " int($12) " " $13 " " $14 " " $15 " " $16 " " $17 " " $18 " " $19 " " $20 " " $21 " " $22 " " $23 " " $24 " " $25 " " $26 " " $27 " " $28 " " $29 " " $30 " " $31 " " $32 " " $33 " " $34 " " $35 " " $36 " " $37 " " $38 " " $39 " " $40 " " $41 " " $42 " " $43 " " $44 " " $45 " " $46 " " $47 " " $48 " " $49 " " $50}' >> $out5

echo "testid msgsize chunksize datasize time bandwidth numpaths/job maxpath/job minpath/job avgpath/job medpath/job totalhopbytes maxhopbytes minhopbytes avghopbytes medhopbytes totalcopies maxcopies/path mincopies/paths avgcopies/paths medcopies/path totalhops maxhops/path minhop/path avghops/path medhop/path totalcopies/node maxopies/node mincopies/node avgcopies/node medcopies/node maxloadpath minloadpath avgloadpath medloadpath maxloadlink minloadlink avgloadlink medloadlink totalrput maxrputs minrput avgrput medrput" >> $out6

grep "OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu16 | awk '{print $2 " " $6 " " $9 " " $10 " " int($11) " " int($12) " " $13 " " $14 " " $15 " " $16 " " $17 " " $18 " " $19 " " $20 " " $21 " " $22 " " $23 " " $24 " " $25 " " $26 " " $27 " " $28 " " $29 " " $30 " " $31 " " $32 " " $33 " " $34 " " $35 " " $36 " " $37 " " $38 " " $39 " " $40 " " $41 " " $42 " " $43 " " $44 " " $45 " " $46 " " $47 " " $48 " " $49 " " $50}' >> $out6

echo "testid msgsize chunksize datasize time bandwidth numpaths/job maxpath/job minpath/job avgpath/job medpath/job totalhopbytes maxhopbytes minhopbytes avghopbytes medhopbytes totalcopies maxcopies/path mincopies/paths avgcopies/paths medcopies/path totalhops maxhops/path minhop/path avghops/path medhop/path totalcopies/node maxopies/node mincopies/node avgcopies/node medcopies/node maxloadpath minloadpath avgloadpath medloadpath maxloadlink minloadlink avgloadlink medloadlink totalrput maxrputs minrput avgrput medrput" >> $out7

grep "OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu32 | awk '{print $2 " " $6 " " $9 " " $10 " " int($11) " " int($12) " " $13 " " $14 " " $15 " " $16 " " $17 " " $18 " " $19 " " $20 " " $21 " " $22 " " $23 " " $24 " " $25 " " $26 " " $27 " " $28 " " $29 " " $30 " " $31 " " $32 " " $33 " " $34 " " $35 " " $36 " " $37 " " $38 " " $39 " " $40 " " $41 " " $42 " " $43 " " $44 " " $45 " " $46 " " $47 " " $48 " " $49 " " $50}' >> $out7
