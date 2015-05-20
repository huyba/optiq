
grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../512/data/opt_mp50.dat > opt_87.dat
grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../1k/data/opt_mp50.dat >> opt_87.dat
grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../2k/data/opt_mp50.dat >> opt_87.dat
grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../4k/data/opt_mp50.dat >> opt_87.dat
grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../8k/data/opt_mp50.dat >> opt_87.dat

grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../512/data/heu_ml4.dat > heu_ml4_87.dat
grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../1k/data/heu_ml4.dat >> heu_ml4_87.dat
grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../2k/data/heu_ml4.dat >> heu_ml4_87.dat
grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../4k/data/heu_ml4.dat >> heu_ml4_87.dat
grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" ../../8k/data/heu_ml4.dat >> heu_ml4_87.dat

grep "M  87 MPI_Alltoallv  msg = 8388608 chunk = 65536" ../../512/data/opt_mp50.dat > mpi_87.dat
grep "M  87 MPI_Alltoallv  msg = 8388608 chunk = 65536" ../../1k/data/opt_mp50.dat >> mpi_87.dat
grep "M  87 MPI_Alltoallv  msg = 8388608 chunk = 65536" ../../2k/data/opt_mp50.dat >> mpi_87.dat
grep "M  87 MPI_Alltoallv  msg = 8388608 chunk = 65536" ../../4k/data/opt_mp50.dat >> mpi_87.dat
grep "M  87 MPI_Alltoallv  msg = 8388608 chunk = 65536" ../../8k/data/heu_ml4.dat >> mpi_87.dat
