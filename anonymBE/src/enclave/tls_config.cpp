#include <tls_config.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <map>
#include <mutex>
#include <enclave_anonymbe_t.h>

#ifdef printf
#undef printf
extern "C" {
extern int printf(const char *fmt, ...);
}
#endif

//------------------------------------------------------------------------------
static int password_cb(char *buf, int size, int rwflag, void *password) {
    strncpy(buf, (char *)(password), size);
    buf[size - 1] = '\0';
    return strlen(buf);
}

//------------------------------------------------------------------------------
static EVP_PKEY *generatePrivateKey() {
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    EVP_PKEY_keygen_init(pctx);
    EVP_PKEY_CTX_set_rsa_keygen_bits(pctx, 2048);
    EVP_PKEY_keygen(pctx, &pkey);
    return pkey;
}

//------------------------------------------------------------------------------
static X509 *generateCertificate(EVP_PKEY *pkey) {
    X509 *x509 = X509_new();
    X509_set_version(x509, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 0);
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), (long)60 * 60 * 24 * 365);
    X509_set_pubkey(x509, pkey);

    X509_NAME *name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC,
                               (const unsigned char *)"CH", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                               (const unsigned char *)"A-SKY", -1, -1, 0);
    X509_set_issuer_name(x509, name);
    X509_sign(x509, pkey, EVP_sha256());
    return x509;
}

//------------------------------------------------------------------------------
static SSL_CTX *create_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLSv1_2_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        printf("Unable to create SSL context");
        exit(EXIT_FAILURE);
    }
    return ctx;
}

//------------------------------------------------------------------------------
static void configure_context(SSL_CTX *ctx) {
    EVP_PKEY *pkey = generatePrivateKey();
    X509 *x509 = generateCertificate(pkey);

    SSL_CTX_use_certificate(ctx, x509);
    SSL_CTX_set_default_passwd_cb(ctx, password_cb);
    SSL_CTX_use_PrivateKey(ctx, pkey);

    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);
}

//------------------------------------------------------------------------------
void init_openssl(SSL_CTX **ctx) {
    OpenSSL_add_ssl_algorithms();
    OpenSSL_add_all_ciphers();
    SSL_load_error_strings();

    printf("%s\n", SSLeay_version(SSLEAY_VERSION));
    *ctx = create_context();
    SSL_CTX_set_ecdh_auto(*ctx, 1);
    configure_context(*ctx);
}

//------------------------------------------------------------------------------
const char *err_str(int e) {
    switch (e) {
        case SSL_ERROR_NONE:
            return "SSL_ERROR_NONE";
        case SSL_ERROR_SSL:
            return "SSL_ERROR_SSL";
        case SSL_ERROR_WANT_READ:
            return "SSL_ERROR_WANT_READ";
        case SSL_ERROR_WANT_WRITE:
            return "SSL_ERROR_WANT_WRITE";
        case SSL_ERROR_WANT_X509_LOOKUP:
            return "SSL_ERROR_WANT_X509_LOOKUP";
        case SSL_ERROR_SYSCALL:
            return "SSL_ERROR_SYSCALL";
        case SSL_ERROR_ZERO_RETURN:
            return "SSL_ERROR_ZERO_RETURN";
        case SSL_ERROR_WANT_CONNECT:
            return "SSL_ERROR_WANT_CONNECT";
        case SSL_ERROR_WANT_ACCEPT:
            return "SSL_ERROR_WANT_ACCEPT";
        default:
            return "UNKKNOWN";
    };
}

//------------------------------------------------------------------------------
std::map<int, SSL *> open_ssl_connections;
std::mutex conn_mutex;
//------------------------------------------------------------------------------
SSL *get_context(int fd) {
    std::lock_guard<std::mutex> lock(conn_mutex);
    auto it = open_ssl_connections.find(fd);
    return it != open_ssl_connections.end() ? it->second : nullptr;
}

//------------------------------------------------------------------------------
int tls_accept(int fd, SSL_CTX *ctx) {
    SSL *cli = SSL_new(ctx);
    SSL_set_fd(cli, fd);
    ERR_clear_error();
    int r = SSL_accept(cli);
    if (r <= 0) {
        SSL_free(cli);
        r = SSL_get_error(cli, r);
        printf("%s: %s\n", err_str(r),
               ERR_error_string(ERR_get_error(), nullptr));
        return -1;
    }
    {
        std::lock_guard<std::mutex> lock(conn_mutex);
        open_ssl_connections[fd] = cli;
    }
    // printf("Connection accepted fd(%d)\n", fd);
    // printf("ciphersuit: %s\n", SSL_get_current_cipher(cli)->name);
    return 0;
}

//------------------------------------------------------------------------------
void tls_finish() {
    std::lock_guard<std::mutex> lock(conn_mutex);
    for (auto &kv : open_ssl_connections) {
        SSL_shutdown(kv.second);
        SSL_free(kv.second);
        int ret;
        ocall_close(&ret,kv.first);
    }
    open_ssl_connections.clear();
}

//------------------------------------------------------------------------------

int ecall_tls_close(int fd) {
    std::lock_guard<std::mutex> lock(conn_mutex);
    auto it = open_ssl_connections.find(fd);
    if (it != open_ssl_connections.end()) {
        SSL_shutdown(it->second);
        SSL_free(it->second);
        open_ssl_connections.erase(it);
        return 0;
    }
    return -1;
}

//------------------------------------------------------------------------------
int ecall_tls_recv(int fd) {
    char read_buf[1000];
    SSL *ssl = get_context(fd);
    if (ssl) {
        int ret;
        do {
            ERR_clear_error();
            ret = SSL_read(ssl, read_buf, sizeof(read_buf));
            if (ret > 0) {
                ecall_query(fd, read_buf, ret);
                // return 0;
            } else {
                if (ret == 0) return -1;
                int r = SSL_get_error(ssl, r);
                // printf(":%s:\n", err_str(r));
                return -2;
            }
        } while (ret > 0);
    }
    return -3;
}

//------------------------------------------------------------------------------
int tls_send(int fd, const char *buff, size_t len) {
    SSL *ssl = get_context(fd);
    if (ssl) {
        int ret = SSL_write(ssl, buff, len);
        if (ret <= 0) {
            printf("ssl_write err: %d\n", ret);
            return -2;
        }
        return 0;
    }
    return -1;
}
