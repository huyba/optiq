#!/bin/sh

# To gather paths of pairs of a job into 1 single file.

for (( i=0; i <= 90; i++ ))
do
    cat test$i\_* > test$i
    rm test$i\_*
done
