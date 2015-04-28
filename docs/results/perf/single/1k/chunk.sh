
grep "chunk = 8192"  $1 > chunk/8k.dat
grep "chunk = 16384" $1 > chunk/16k.dat
grep "chunk = 32768" $1 > chunk/32k.dat
grep "chunk = 65536" $1 > chunk/64k.dat
grep "chunk = 131072" $1 > chunk/128k.dat
grep "chunk = 262144" $1 > chunk/256k.dat
grep "chunk = 524288" $1 > chunk/512k.dat
grep "chunk = 1048576" $1 > chunk/1m.dat
grep "chunk = 2097152" $1 > chunk/2m.dat
grep "chunk = 4194304" $1 > chunk/4m.dat
grep "chunk = 8388608" $1 > chunk/8m.dat
