
./a.out ./data/graph.dat 64 > 256to4_64.dat

cp 256to4_64.dat ../../../model/path_based/

./a.out ./data/graph.dat 32 > 256to4_32.dat

cp 256to4_32.dat ../../../model/path_based/
