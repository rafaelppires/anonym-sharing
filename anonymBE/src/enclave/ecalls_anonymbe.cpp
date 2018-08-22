#include <enclave_anonymbe_t.h>
#include <anonymbe_service.h>
#include <sgx_cryptoall.h>
#include <stdio.h>

#ifdef MEMDATABASE
#include <memory_database.h>
AnonymBE<MemDatabase> anonymbe;
#else
#include <MongoDatabase.h>
AnonymBE<MongoDatabase> anonymbe;
#endif

//------------------------------------------------------------------------------
int ecall_query( int fd, const char *buff, size_t len ) {
    std::string response;
    anonymbe.process_input( response, buff, len );
    ssize_t ret;
    ocall_send( &ret, fd, response.c_str(), response.size(), 0 );
    return 0;
}

//------------------------------------------------------------------------------
int ecall_init() {
    return anonymbe.init();
}

//------------------------------------------------------------------------------
void ecall_finish() {
    anonymbe.finish();
}

