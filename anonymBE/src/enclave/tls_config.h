#ifndef _TLS_CONFIG_H_DEFINED_
#define _TLS_CONFIG_H_DEFINED_

#include <libc_mock/libc_proxy.h>
#include <openssl/ssl.h>

void init_openssl(SSL_CTX **ctx);
int tls_accept(int fd, SSL_CTX *ctx);
int tls_send(int fd, const char *buff, size_t len);

#endif
