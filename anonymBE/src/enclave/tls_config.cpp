#include <tls_config.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <map>
#include <mutex>
#ifdef NATIVE
#include <ecalls_anonymbe.h>
#include <unistd.h>
#else
#include <enclave_anonymbe_t.h>
#include <my_wrappers.h>
#endif

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
// INCOME SSL CONNECTION
//------------------------------------------------------------------------------
std::map<int, IncomeSSLConnection> IncomeSSLConnection::connection_table;
std::mutex IncomeSSLConnection::table_lock;
SSL_CTX *IncomeSSLConnection::ssl_ctx = nullptr;
//------------------------------------------------------------------------------
IncomeSSLConnection::IncomeSSLConnection(int fd, SSL *ssl)
    : fd_(fd), ssl_(ssl) {}
//------------------------------------------------------------------------------
IncomeSSLConnection::~IncomeSSLConnection() {
    close();
}

//------------------------------------------------------------------------------
IncomeSSLConnection::IncomeSSLConnection(IncomeSSLConnection &&c)
    : fd_(c.fd_), ssl_(c.ssl_), decoder_(std::move(c.decoder_)) {
    c.fd_ = -1;
    c.ssl_ = nullptr;
}

//------------------------------------------------------------------------------
IncomeSSLConnection &IncomeSSLConnection::operator=(IncomeSSLConnection &&c) {
    close();
    fd_ = c.fd_;
    ssl_ = c.ssl_;
    decoder_ = std::move(c.decoder_);
    c.fd_ = -1;
    c.ssl_ = nullptr;
    return *this;
}

//------------------------------------------------------------------------------
void IncomeSSLConnection::init() {
    OpenSSL_add_ssl_algorithms();
    OpenSSL_add_all_ciphers();
    SSL_load_error_strings();

    printf("%s\n", SSLeay_version(SSLEAY_VERSION));
    ssl_ctx = create_context();
    SSL_CTX_set_ecdh_auto(ssl_ctx, 1);
    configure_context(ssl_ctx);
}

//------------------------------------------------------------------------------
int IncomeSSLConnection::addConnection(int fd) {
    if (ssl_ctx == nullptr) return -1;

    {   // may possibly happen when a fd recently closed is re-assigned to 
        // a new socket
        std::lock_guard<std::mutex> lock(table_lock);
        if (connection_table.find(fd) != connection_table.end())
            connection_table.erase(fd);
    }

    SSL *cli = SSL_new(ssl_ctx);
    SSL_set_fd(cli, fd);
    ERR_clear_error();
    int r = SSL_accept(cli);
    if (r <= 0) {
        SSL_free(cli);
        r = ERR_get_error();
        printf("accept err fd: %d %s: %s\n", fd, err_str(SSL_get_error(cli, r)),
               r ? ERR_error_string(r, nullptr) : "");
        ::close(fd);
        return -2;
    }
    {
        std::lock_guard<std::mutex> lock(table_lock);
        connection_table.insert(
            std::make_pair(fd, IncomeSSLConnection(fd, cli)));
    }
    // printf("Connection accepted fd(%d)\n", fd);
    // printf("ciphersuit: %s\n", SSL_get_current_cipher(cli)->name);
    return 0;
}

//------------------------------------------------------------------------------
IncomeSSLConnection &IncomeSSLConnection::getConnection(int fd) {
    std::lock_guard<std::mutex> lock(table_lock);
    auto it = connection_table.find(fd);
    if (it != connection_table.end())
        return it->second;
    else
        throw NOT_FOUND;
}

//------------------------------------------------------------------------------
void IncomeSSLConnection::removeConnection(int fd) {
    std::lock_guard<std::mutex> lock(table_lock);
    auto it = connection_table.find(fd);
    if (it != connection_table.end())
        connection_table.erase(it);
    else
        throw NOT_FOUND;
}

//------------------------------------------------------------------------------
void IncomeSSLConnection::finish() {
    std::lock_guard<std::mutex> lock(table_lock);
    for (auto &kv : connection_table) {
        kv.second.close();
    }
    connection_table.clear();
}

//------------------------------------------------------------------------------
int IncomeSSLConnection::recv(char *buff, size_t len) {
    int read = SSL_read(ssl_, buff, len);
    if (read == 0) {
        close();
        throw CONNECTION_CLOSED;
    } else if (read < 0) {
        throw std::runtime_error(err_str(SSL_get_error(ssl_, read)));
    }
    return read;
}

//------------------------------------------------------------------------------
int IncomeSSLConnection::send(const char *buff, size_t len) {
    int ret = SSL_write(ssl_, buff, len);
    if (ret <= 0) {
        printf("ssl_write err: %d\n", ret);
        return -1;
    }
    return 0;
}

//------------------------------------------------------------------------------
void IncomeSSLConnection::close() {
    if (ssl_ != nullptr) {
        // SSL_shutdown(ssl_);
        SSL_free(ssl_);
        ssl_ = nullptr;
    }

    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}

//------------------------------------------------------------------------------
extern void process_input(IncomeSSLConnection &conn, const char *buff,
                          size_t len);
char read_buf[1024 * 1024];
int ecall_tls_recv(int fd) {
    try {
        IncomeSSLConnection &conn = IncomeSSLConnection::getConnection(fd);
        int ret;
        do {
            ret = conn.recv(read_buf, sizeof(read_buf));
            process_input(conn, read_buf, ret);
        } while (ret > 0);
    } catch (IncomeSSLConnection::Exceptions e) {
        switch (e) {
            case IncomeSSLConnection::NOT_FOUND:
                return -1;
            case IncomeSSLConnection::CONNECTION_CLOSED:
                IncomeSSLConnection::removeConnection(fd);
                return -2;
            default:
                return -3;
        }
    } catch (const std::exception &e) {
        std::string msg(e.what());
        if (msg != "SSL_ERROR_WANT_READ" && msg != "SSL_ERROR_SYSCALL")
            printf("%s\n", e.what());
        return -4;
    }
    return 0;
}

//------------------------------------------------------------------------------
