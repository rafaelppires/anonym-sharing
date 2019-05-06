#!/bin/zsh

date=$(date +%Y-%m-%d-%H%M)

for scale in $(seq 1 1 10)
do
	threads=$(echo "$scale * 4" | bc)
	java -cp build/libs/anonymbe-client-0.6.jar org.openjdk.jmh.Main -bm thrpt -t $threads -to 300s -r 10s -wi 0 -i 4 -f 3 -p scale=$scale -p groupSize=1,10,100,1000,10000 -rff ~/envelope-$date-s$scale.csv createEnvelopeBenchmarkTput 2>&1 | tee ~/envelope-log-$date-s$scale.txt
done
