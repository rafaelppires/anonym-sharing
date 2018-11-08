#include <enclave_writeproxy_t.h>
#include <writeproxy_args.h>

int ecall_init(struct Arguments args) { return 0; }

void ecall_finish() {  }

int ecall_tls_accept(int fd) { return 0; }

int ecall_query(int fd,const char *buff, size_t len) {
    return 0;
}

