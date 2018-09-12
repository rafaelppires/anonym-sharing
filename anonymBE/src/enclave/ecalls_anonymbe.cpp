#include <anonymbe_service.h>
#include <enclave_anonymbe_t.h>
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
int ecall_query(int fd, const char *buff, size_t len) {
    std::string response;
    anonymbe.process_input(response, buff, len);
    ssize_t ret;
#ifdef TLS_REQUESTS
    int r = tls_send(fd, response.c_str(), response.size());
#else
    ocall_send(&ret, fd, response.c_str(), response.size(), 0);
#endif
    return 0;
}

//------------------------------------------------------------------------------
int ecall_init() { return anonymbe.init(); }

//------------------------------------------------------------------------------
int ecall_tls_accept(int fd) { return anonymbe.accept(fd);}

//------------------------------------------------------------------------------
void ecall_finish() { anonymbe.finish(); }
