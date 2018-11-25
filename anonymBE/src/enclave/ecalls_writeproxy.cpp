#include <enclave_writeproxy_t.h>
#include <http1decoder.h>
#include <httpcommon.h>
#include <httpheaders.h>
#include <httpresponse.h>
#include <minioclient.h>
#include <tls_config.h>
#include <writeproxy_args.h>
#include <string>

extern "C" {
extern int printf(const char *fmt, ...);
}
SSL_CTX *ssl_context;
Http1Decoder decoder;
MinioClient *minioClient = nullptr;
HeadersBuilder header_builder;
ResponseBuilder response_builder;
std::basic_ostream<char> cout;
//------------------------------------------------------------------------------
int ecall_init(struct Arguments args) {
    init_openssl(&ssl_context);
    header_builder.set(HttpStrings::user_agent, "A-SKY WriterProxy")
        .set(HttpStrings::connection, HttpStrings::keepalive);
    response_builder.protocol(HttpStrings::http11).headers(header_builder);
    minioClient =
        new MinioClient("http://127.0.0.1:9000", "IXAFVR4LKA12P2FACHT8",
                        "RJR0W75O7Y27lzdBkY8LoymxWNE2ecdB0MqRLZQY");
    // minioClient->traceOn(cout);
    return 0;
}

//------------------------------------------------------------------------------
void ecall_finish() {
    delete minioClient;
    tls_finish();
}

//------------------------------------------------------------------------------
int ecall_tls_accept(int fd) { return tls_accept(fd, ssl_context); }

//------------------------------------------------------------------------------
bool make_bucket(const Request &request) {
    const HttpUrl &url = request.url();
    std::string path = url.encodedPath(), bucket;
    auto pieces = split(path, "/");
    if (!pieces.empty()) bucket = pieces[0];
    if (bucket.empty()) return false;
    try {
        minioClient->makeBucket(bucket);
    } catch (const std::exception &e) {
        printf("Error creating bucket: %d\n", e.what());
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
Response forward(const Request &request) {
    try {
        minioClient->forwardObject(request);
    } catch (const ErrorResponseException &e) {
        if (e.code() == ErrorCode::NO_SUCH_BUCKET && make_bucket(request)) {
            forward(request);
        } else {
            response_builder.code(409).message("Conflict");
            return response_builder.build();
        }
    } catch (const std::exception &e) {
        //printf("Error: (%s)\n", e.what());
        std::string norep = "No response";
        if (norep == e.what()) {
            response_builder.code(444).message(norep);
            return response_builder.build();
        }
    }
    response_builder.code(200).message("OK");
    return response_builder.build();
}

//------------------------------------------------------------------------------
int requests_received = 0;
int ecall_query(int fd, const char *buff, size_t len) {
    Request req;
    std::string input(buff, len);

#if 0
    const std::string eol = "\r\n", posrep = "HTTP/1.1 200 OK" + eol +
                                             "Connection: Keep-Alive" + eol +
                                             "Content-Type: application/json" +
                                             eol + "Content-Length: 0" + eol +
                                             eol;
#endif
    try {
        decoder.addChunk(input);
        while (decoder.requestReady()) {
            ++requests_received;
            if (requests_received % 100000 == 0)
                printf("%luk Requests\n", requests_received / 1000);
            req = decoder.getRequest();
            std::string response = forward(req).toString();
            // printf("(REP{%s}REP)\n", response.c_str());
            if (tls_send(fd, response.data(), response.size()) < 0)
                // if (tls_send(fd, posrep.data(), posrep.size()) < 0)
                printf("not sent\n");
        }
    } catch (const std::exception &e) {
        printf("Error: [%s]\n", e.what());
    }
    return 0;
}

//------------------------------------------------------------------------------
