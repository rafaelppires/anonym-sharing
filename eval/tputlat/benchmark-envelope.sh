#!/bin/zsh

timestamp=$(date +%Y%m%d-%H%M)
groupsize=$4
output=results/envelope-$timestamp-group$groupsize.txt

for rate in $(seq $1 $2 $3)
do
    ./wrk2/wrk -t 1 -s envelope.lua -d 5s -R $rate https://hoernli-4.maas:30444/verifier/envelope -- $groupsize | tee -a $output
done

