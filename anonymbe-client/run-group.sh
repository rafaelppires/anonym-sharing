#!/bin/zsh

date=$(date +%Y-%m-%d-%H%M)

for scale in $(seq 1 1 10)
do
	threads=$(echo "$scale * 4" | bc)
	java -cp build/libs/anonymbe-client-0.6.jar org.openjdk.jmh.Main -bm thrpt -t $threads -to 2m -r 5s -wi 0 -i 1 -f 20 -p scale=$scale -p usersPreadded=1,10,100,1000,10000 -rff ~/group-$date-s$scale.csv addUserToGroupBenchmark$ 2>&1 | tee ~/group-log-$date-s$scale.txt
done
