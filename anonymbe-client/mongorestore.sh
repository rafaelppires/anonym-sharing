docker run --rm --network=host --volume=/home/ubuntu/mongobackup:/mongobackup:ro mongo:4.0.9-xenial mongorestore --host=rs0/localhost:30000 -d newtest --ssl --sslAllowInvalidCertificates --sslAllowInvalidHostnames --gzip --archive=/mongobackup/envelope-state.mongo.gz