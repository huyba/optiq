grep "O  27 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../512/data/opt_mp50.dat > opt_27.dat
grep "O  27 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../1k/data/opt_mp50.dat >> opt_27.dat
grep "O  27 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../2k/data/opt_mp50.dat >> opt_27.dat
grep "O  27 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../4k/data/opt_mp50.dat >> opt_27.dat
grep "O  27 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../8k/data/opt_mp50.dat >> opt_27.dat

grep "O  27 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../512/data/heu_ml4.dat > heu_ml4_27.dat
grep "O  27 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../1k/data/heu_ml4.dat >> heu_ml4_27.dat
grep "O  27 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../2k/data/heu_ml4.dat >> heu_ml4_27.dat
grep "O  27 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../4k/data/heu_ml4.dat >> heu_ml4_27.dat
grep "O  27 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../8k/data/heu_ml4.dat >> heu_ml4_27.dat

grep "M  27 MPI_Alltoallv  msg = 8388608 chunk = 65536" ../../512/data/opt_mp50.dat > mpi_27.dat
grep "M  27 MPI_Alltoallv  msg = 8388608 chunk = 65536" ../../1k/data/opt_mp50.dat >> mpi_27.dat
grep "M  27 MPI_Alltoallv  msg = 8388608 chunk = 65536" ../../2k/data/opt_mp50.dat >> mpi_27.dat
grep "M  27 MPI_Alltoallv  msg = 8388608 chunk = 65536" ../../4k/data/opt_mp50.dat >> mpi_27.dat
grep "M  27 MPI_Alltoallv  msg = 8388608 chunk = 65536" ../../8k/data/heu_ml4.dat >> mpi_27.dat
