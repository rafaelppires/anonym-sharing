#include <anonymbe_service.h>
#ifndef NATIVE
#include <enclave_anonymbe_t.h>
#endif
#include <http1decoder.h>
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

std::mutex table_lock;
std::map<int, Http1Decoder> decoder_table;
//------------------------------------------------------------------------------
long unsigned  requests_received = 0;
int ecall_query(int fd, const char *buff, size_t len) {
    try {
        std::string input(buff, len), response;
        Request req;
        Http1Decoder *dec_ptr = nullptr;
        {
            std::lock_guard<std::mutex> lock(table_lock);
            dec_ptr = &decoder_table[fd];
        }
        Http1Decoder &decoder = *dec_ptr;

        decoder.addChunk(input);
        while (decoder.requestReady()) {
            if (++requests_received % 100000 == 0)
                printf("%luk Requests\n", requests_received / 1000);
            req = decoder.getRequest();
            response = anonymbe.process_input(req).toString();
            ssize_t ret;
#ifdef TLS_REQUESTS
            int r = tls_send(fd, response.data(), response.size());
#else
            ocall_send(&ret, fd, response.data(), response.size(), 0);
#endif
        }
    } catch (const std::exception &e) {
        printf("Error: [%s]\n", e.what());
        return -1;
    } catch (...) {
        printf(":_(   o.O   ;_;\n");
        return -2;
    }
    return 0;
}

//------------------------------------------------------------------------------
int ecall_init(struct Arguments args) { return anonymbe.init(&args); }

//------------------------------------------------------------------------------
int ecall_tls_accept(int fd) { return anonymbe.accept(fd); }

//------------------------------------------------------------------------------
int ecall_tls_close(int fd) {
    // printf("Closed file [%d]\n", fd);
    {
        std::lock_guard<std::mutex> lock(table_lock);
        decoder_table.erase(fd);
    }
    return tlsserver_close(fd);
}

//------------------------------------------------------------------------------
void ecall_finish() { anonymbe.finish(); }

//------------------------------------------------------------------------------
