#!/bin/zsh

curl -k -X POST --data '{"user_id":"bla"}' https://127.0.0.1:4444/access/user
curl -k -X POST --data '{"group_name":"groupname","user_id":"bla"}' https://127.0.0.1:4444/access/group

for i in `seq 1000 1000 10000`
do
	./wrk --latency -s post.lua -d 5 -R $i https://localhost:4444/ | tee -a standard-devserver-http.log
done
