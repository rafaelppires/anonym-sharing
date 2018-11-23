#include <enclave_writeproxy_t.h>
#include <http1decoder.h>
#include <httpheaders.h>
#include <httpresponse.h>
#include <tls_config.h>
#include <writeproxy_args.h>
#include <string>
#include <minioclient.h>

extern "C" {
extern int printf(const char *fmt, ...);
}
SSL_CTX *ssl_context;
Http1Decoder decoder;
MinioClient *minioClient;

//------------------------------------------------------------------------------
int ecall_init(struct Arguments args) {
    init_openssl(&ssl_context);
    minioClient = new MinioClient("http://127.0.0.1:9000", 
                                  "Y87GXVKRA7JUGN705P9V",
                                  "tC5AdH3WI5CDGflF2gdSRqEvtqslgMNakiCDpFg+");
    return 0;
}

//------------------------------------------------------------------------------
void ecall_finish() {}

//------------------------------------------------------------------------------
int ecall_tls_accept(int fd) { return tls_accept(fd, ssl_context); }

//------------------------------------------------------------------------------
int ecall_query(int fd, const char *buff, size_t len) {
    Request req;
    std::string input(buff, len);
    try {
        decoder.addChunk(input);
        if (decoder.requestReady()) {
            req = decoder.getRequest();
            printf("<%s>\n", (req.statusLine() + "\n" +
                              req.headers().toString() + req.stringBody())
                                 .c_str());
        }
    } catch (const std::exception &e) {
        printf("Error: [%s]\nInput:[%s]\n", e.what(), input.c_str());
    }
    return 0;
}

//------------------------------------------------------------------------------
