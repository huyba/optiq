id=$1

inopt=data/opt_mp50.dat
outopt=test$id\_opt.dat
outmpi=test$id\_mpi.dat

inheu1=data/heu_ml1.dat
outheu1=test$id\_heu_ml1.dat

inheu2=data/heu_ml2.dat
outheu2=test$id\_heu_ml2.dat

inheu4=data/heu_ml4.dat
outheu4=test$id\_heu_ml4.dat

inheu8=data/heu_ml8.dat
outheu8=test$id\_heu_ml8.dat

inheu16=data/heu_ml16.dat
outheu16=test$id\_heu_ml16.dat

inheu32=data/heu_ml32.dat
outheu32=test$id\_heu_ml32.dat

grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../512/$inopt | awk '{print  "512 " $10 " " $11 " " $12}' > $outopt
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../1k/$inopt | awk '{print  "1024 " $10 " " $11 " " $12}' >> $outopt
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../2k/$inopt | awk '{print  "2048 " $10 " " $11 " " $12}' >> $outopt
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../4k/$inopt | awk '{print  "4092 " $10 " " $11 " " $12}' >> $outopt
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../8k/$inopt | awk '{print  "8192 " $10 " " $11 " " $12}' >> $outopt

grep "M  $id MPI_Alltoallv  msg = 8388608 chunk = 65536" ../512/$inopt | awk '{print  "512 " $10 " " $11 " " $12}' > $outmpi
grep "M  $id MPI_Alltoallv  msg = 8388608 chunk = 65536" ../1k/$inopt | awk '{print  "1024 " $10 " " $11 " " $12}' >> $outmpi
grep "M  $id MPI_Alltoallv  msg = 8388608 chunk = 65536" ../2k/$inopt | awk '{print  "2048 " $10 " " $11 " " $12}' >> $outmpi
grep "M  $id MPI_Alltoallv  msg = 8388608 chunk = 65536" ../4k/$inopt | awk '{print  "4096 " $10 " " $11 " " $12}' >> $outmpi
grep "M  $id MPI_Alltoallv  msg = 8388608 chunk = 65536" ../8k/$inheu4 | awk '{print  "8192 " $10 " " $11 " " $12}' >> $outmpi

grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../512/$inheu1 | awk '{print  "512 " $10 " " $11 " " $12}' > $outheu1
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../1k/$inheu1 | awk '{print  "1024 " $10 " " $11 " " $12}' >> $outheu1
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../2k/$inheu1 | awk '{print  "2048 " $10 " " $11 " " $12}' >> $outheu1
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../4k/$inheu1 | awk '{print  "4096 " $10 " " $11 " " $12}' >> $outheu1
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../8k/$inheu1 | awk '{print  "8192 " $10 " " $11 " " $12}' >> $outheu1

grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../512/$inheu2 | awk '{print  "512 " $10 " " $11 " " $12}' > $outheu2
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../1k/$inheu2 | awk '{print  "1024 " $10 " " $11 " " $12}' >> $outheu2
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../2k/$inheu2 | awk '{print  "2048 " $10 " " $11 " " $12}' >> $outheu2
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../4k/$inheu2 | awk '{print  "4096 " $10 " " $11 " " $12}' >> $outheu2
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../8k/$inheu2 | awk '{print  "8192 " $10 " " $11 " " $12}' >> $outheu2

grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../512/$inheu4 | awk '{print  "512 " $10 " " $11 " " $12}' > $outheu4
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../1k/$inheu4 | awk '{print  "1024 " $10 " " $11 " " $12}' >> $outheu4
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../2k/$inheu4 | awk '{print  "2048 " $10 " " $11 " " $12}' >> $outheu4
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../4k/$inheu4 | awk '{print  "4096 " $10 " " $11 " " $12}' >> $outheu4
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../8k/$inheu4 | awk '{print  "8192 " $10 " " $11 " " $12}' >> $outheu4

grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../512/$inheu8 | awk '{print  "512 " $10 " " $11 " " $12}' > $outheu8
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../1k/$inheu8 | awk '{print  "1024 " $10 " " $11 " " $12}' >> $outheu8
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../2k/$inheu8 | awk '{print  "2048 " $10 " " $11 " " $12}' >> $outheu8
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../4k/$inheu8 | awk '{print  "4096 " $10 " " $11 " " $12}' >> $outheu8
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../8k/$inheu8 | awk '{print  "8192 " $10 " " $11 " " $12}' >> $outheu8

grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../512/$inheu16 | awk '{print  "512 " $10 " " $11 " " $12}' > $outheu16
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../1k/$inheu16 | awk '{print  "1024 " $10 " " $11 " " $12}' >> $outheu16
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../2k/$inheu16 | awk '{print  "2048 " $10 " " $11 " " $12}' >> $outheu16
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../4k/$inheu16 | awk '{print  "4096 " $10 " " $11 " " $12}' >> $outheu16
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../8k/$inheu16 | awk '{print  "8192 " $10 " " $11 " " $12}' >> $outheu16

grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../512/$inheu32 | awk '{print  "512 " $10 " " $11 " " $12}' > $outheu32
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../1k/$inheu32 | awk '{print  "1024 " $10 " " $11 " " $12}' >> $outheu32
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../2k/$inheu32 | awk '{print  "2048 " $10 " " $11 " " $12}' >> $outheu32
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../4k/$inheu32 | awk '{print  "4096 " $10 " " $11 " " $12}' >> $outheu32
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../8k/$inheu32 | awk '{print  "8192 " $10 " " $11 " " $12}' >> $outheu32
