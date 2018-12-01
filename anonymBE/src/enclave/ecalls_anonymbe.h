#ifdef NATIVE
#ifndef _ECALLS_ANONYMBE_H_
#define _ECALLS_ANONYMBE_H_

#include <cstdlib>

int ecall_init(struct Arguments args);
void ecall_finish();
int ecall_query(int fd, const char *buff, size_t len);
int ecall_tls_recv(int fd);
int ecall_tls_close(int fd);
int ecall_tls_accept(int fd);

#endif //_ECALLS_ANONYMBE_H_
#endif // NATIVE

