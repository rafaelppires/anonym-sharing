#include <enclave_anonymbe_t.h>
#include <anonymbe_service.h>
#include <sgx_cryptoall.h>
#include <stdio.h>

#ifdef MEMDATABASE
#include <memory_database.h>
#else
#include <MongoDatabase.h>
#endif

//------------------------------------------------------------------------------
int printf(const char *fmt, ...) {
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    int ret = vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print(buf);
    return ret;
}
//------------------------------------------------------------------------------

#ifdef MEMDATABASE
AnonymBE<MDatabase> anonymbe;
#else
AnonymBE<MongoDatabase> anonymbe;
#endif

//------------------------------------------------------------------------------
int ecall_query( int fd, const char *buff, size_t len ) {
    std::string response;
    anonymbe.process_input( response, buff, len );
    ocall_response( fd, response.c_str(), response.size() );
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

