#ifndef _TLS_CONFIG_H_DEFINED_
#define _TLS_CONFIG_H_DEFINED_

#ifndef NATIVE
#include <libc_mock/libc_proxy.h>
#endif
#include <openssl/ssl.h>
#include <http1decoder.h>
#include <mutex>

class IncomeSSLConnection {
public:
    IncomeSSLConnection(int fd, SSL *ssl);
    ~IncomeSSLConnection();
    IncomeSSLConnection(IncomeSSLConnection &&);
    IncomeSSLConnection& operator=(IncomeSSLConnection &&);
    Http1Decoder& decoder() { return decoder_; }

    int send(const char *buff, size_t len);
    int recv(char *buff, size_t len);
    void close();

    enum Exceptions { NO_ERROR, NOT_FOUND, CONNECTION_CLOSED };

    static void init();
    static int addConnection(int fd);
    static IncomeSSLConnection& getConnection(int fd);
    static void removeConnection(int fd);
    static void finish();

private:
    static std::mutex table_lock;
    static std::map<int, IncomeSSLConnection> connection_table;
    static SSL_CTX *ssl_ctx;

    int fd_;
    SSL *ssl_;
    Http1Decoder decoder_;
};

#endif
