## To compile with MongoDB
```
$ make
```
## To compile with in-memory DB
``
$ make MEMDATABASE=1
``
Note: In order to generate some dependency source files, you need first to do:
```
$ make -C src/mongo-sgx-client/ SGX_MODE=HW
```
This command will fail, but it will generate the files we need nevertheless.
TODO: integrate that in Makefile

## To test 
```
$ ./bin/anonymbe
$ ./src/admin/admin.py 
```

