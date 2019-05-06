#!/bin/zsh

timestamp=$(date +%Y%m%d-%H%M)
groupsize=$4
output=results/envelope-$timestamp-group$groupsize.txt

for rate in $(seq $1 $2 $3)
do
    ./wrk2/wrk --timeout 5000 -t 1 -s envelope.lua -d 15s -R $rate https://enp2s0f0.eiger-10.maas:30444/verifier/envelope -- $groupsize | tee -a $output
done

