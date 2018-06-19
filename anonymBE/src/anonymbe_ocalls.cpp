#include <enclave_anonymbe_u.h>
#include <stdio.h>
#include <sys/socket.h>

//------------------------------------------------------------------------------
void ocall_response( int fd, const char *buff, size_t len ) {
    send( fd, buff, len, 0 );
}

//------------------------------------------------------------------------------
void ocall_print( const char* str ) {
    printf("\033[96m%s\033[0m", str);
}

