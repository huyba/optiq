
grep "chunk = 8192"  $1 > $2/8k.dat
grep "chunk = 16384" $1 > $2/16k.dat
grep "chunk = 32768" $1 > $2/32k.dat
grep "chunk = 65536" $1 > $2/64k.dat
grep "chunk = 131072" $1 > $2/128k.dat
grep "chunk = 262144" $1 > $2/256k.dat
grep "chunk = 524288" $1 > $2/512k.dat
grep "chunk = 1048576" $1 > $2/1m.dat
grep "chunk = 2097152" $1 > $2/2m.dat
grep "chunk = 4194304" $1 > $2/4m.dat
grep "chunk = 8388608" $1 > $2/8m.dat
