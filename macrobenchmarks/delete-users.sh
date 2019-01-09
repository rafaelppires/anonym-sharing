#!/bin/zsh

echo "Removing all users from database"
curl -v --insecure -X DELETE https://hoernli-4.maas:30444/access/all
