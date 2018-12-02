#!/bin/zsh

for i in `seq 100 100 1000`
do
	./wrk --latency -s post.lua -d 5 -R $i https://localhost:4445/ | tee -a standard-devserver-http.log
done
