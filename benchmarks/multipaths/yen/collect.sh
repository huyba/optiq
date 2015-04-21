#!/bin/sh

for (( i=0; i <= 90; i++ ))
do
    cat test$i\_* > test$i
    rm test$i\_*
done
