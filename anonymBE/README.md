## To compile with MongoDB
```
$ make
```
## To compile with in-memory DB
```
$ make MEMDATABASE=1
```
Note: In order to generate some dependency source files, you need first to do:
```
$ make -C src/mongo-sgx-client/ SGX_MODE=HW
```
This command will fail, but it will generate the files we need anyhow.
TODO: integrate that in Makefile

## To test 
```
$ ./bin/anonymbe
$ ./src/admin/admin.py 
```

## Deploying to Kubernetes

1. Build Docker image
2. Publish Docker image somewhere accessible to the Kubernetes nodes
3. Make sure that you have started the MongoDB StatefulSet before
4. Create the deployment
```
kubectl apply -f anonymbe.yml
```
5. Query the server from outside Kubernetes on any physical node on port 30444
