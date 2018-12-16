#ifndef NATIVE
#include <enclave_writeproxy_t.h>
#endif
#include <http1decoder.h>
#include <tls_config.h>
#include <string>
#include <writerproxy_service.h>

extern "C" {
extern int printf(const char *fmt, ...);
}
SSL_CTX *ssl_context;
#ifndef NATIVE
std::basic_ostream<char> cout;
#endif

WriterProxy writerproxy;
//------------------------------------------------------------------------------
int ecall_init(struct Arguments args) {
    return writerproxy.init(args);
}

//------------------------------------------------------------------------------
void ecall_finish() {
    writerproxy.finish();
}

//------------------------------------------------------------------------------
int ecall_tls_accept(int fd) { return IncomeSSLConnection::addConnection(fd); }

//------------------------------------------------------------------------------
long unsigned requests_received = 0;
void process_input(IncomeSSLConnection &conn, const char *buff, size_t len) {
    Request req;
    std::string input(buff, len);

    Http1Decoder &decoder = conn.decoder();

    try {
        decoder.addChunk(input);
        while (decoder.requestReady()) {
            if (++requests_received % 100000 == 0)
                printf("%luk Requests\n", requests_received / 1000);
            req = decoder.getRequest();
            std::string response = writerproxy.treat_request(req).toString();
            if (conn.send(response.data(), response.size()) < 0)
                printf("not sent\n");
        }
    } catch (const std::exception &e) {
        printf("Error: [%s]\n", e.what());
    }
}

//------------------------------------------------------------------------------
