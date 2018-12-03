#include <anonymbe_service.h>
#ifndef NATIVE
#include <enclave_anonymbe_t.h>
#endif
#include <sgx_cryptoall.h>
#include <stdio.h>
#include <mutex>

#ifdef MEMDATABASE
#include <memory_database.h>
AnonymBE<MemDatabase> anonymbe;
#else
#include <MongoDatabase.h>
AnonymBE<MongoDatabase> anonymbe;
#endif

//------------------------------------------------------------------------------
long unsigned requests_received = 0;
void process_input(IncomeSSLConnection &conn, const char *buff, size_t len) {
    try {
        std::string input(buff, len), response;
        Request req;

        Http1Decoder &decoder = conn.decoder();
        decoder.addChunk(input);
        while (decoder.requestReady()) {
            if (++requests_received % 100000 == 0)
                printf("%luk Requests\n", requests_received / 1000);
            req = decoder.getRequest();
            response = anonymbe.process_input(req).toString();
            ssize_t ret;
#ifdef TLS_REQUESTS
            int r = conn.send(response.data(), response.size());
#else
            ocall_send(&ret, fd, response.data(), response.size(), 0);
#endif
        }
    } catch (const std::exception &e) {
        printf("Error: [%s]\n", e.what());
    } catch (...) {
        printf(":_(   o.O   ;_;\n");
    }
}

//------------------------------------------------------------------------------
int ecall_init(struct Arguments args) {
    IncomeSSLConnection::init();
    return anonymbe.init(&args);
}

//------------------------------------------------------------------------------
int ecall_tls_accept(int fd) { return IncomeSSLConnection::addConnection(fd); }

//------------------------------------------------------------------------------
void ecall_finish() {
    IncomeSSLConnection::finish();
    anonymbe.finish();
}

//------------------------------------------------------------------------------
