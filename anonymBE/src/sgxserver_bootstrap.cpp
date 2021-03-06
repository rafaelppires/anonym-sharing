#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <csignal>
#include <fstream>
#include <iostream>
#include <queue>
#include <streambuf>
#include <thread>

#ifdef NATIVE
#include <ecalls_anonymbe.h>
#else
#include <sgx_initenclave.h>
#endif
#include <sgx_utils_rp.h>
#include <event_loop.h>

#define ENCLAVEFILE ENCLAVENAME ".signed.so"
#define TOKENFILE   ENCLAVENAME ".token"

#ifndef NATIVE
sgx_enclave_id_t g_eid;
#endif

std::condition_variable iqcv, oqcv;
std::mutex iqmutex, oqmutex;

FdQueue iqueue;  // ready file descriptors
DataQueue oqueue;

//------------------------------------------------------------------------------
void enclave_thread(int id, const SocketEventLoop& comm) {
    signal(SIGPIPE,SIG_IGN);
    for (;;) {
        int fd = -1;
        {  // sleeps until there is data
            std::unique_lock<std::mutex> lk(iqmutex);
            iqcv.wait(lk, [] { return !iqueue.empty(); });
            fd = iqueue.front();
            iqueue.pop();
        }

        std::string msg;
        comm.consume_fd(fd, msg);

#ifndef TLS_REQUESTS
        if (!msg.empty()) {
            int ret;
#ifdef NATIVE
            ret = ecall_query(fd, msg.c_str(), msg.size());
#else
            ecall_query(g_eid, &ret, fd, msg.c_str(), msg.size());
#endif
        }
#endif
    }
}

//------------------------------------------------------------------------------
void sigpipe_handler(int s) {
    printf("SIGPIPE\n");
}

//------------------------------------------------------------------------------
void ctrlc_handler(int s) {
    printf("\033[0m\n(%d) bye!\n", s);
#ifdef NATIVE
    ecall_finish();
#else
    ecall_finish(g_eid);
    destroy_enclave(g_eid);
#endif
    exit(0);
}

//------------------------------------------------------------------------------
int main(int argc, char** argv) {
    signal(SIGPIPE,SIG_IGN);

    Arguments args; 
    init_args(&args);
    struct argp argp = { options, parse_opt, 0, doc };
    argp_parse(&argp, argc, argv, 0, 0, &args);

    /* Changing dir to where the executable is.*/
    change_dir(argv[0]);

#ifndef NATIVE
    // Sets up enclave and initializes remote_attestation
    if (initialize_enclave(g_eid, ENCLAVEFILE, TOKENFILE)) {
        return 1;
    }
#endif

    int ret;
#ifdef NATIVE
    ret = ecall_init(args);
#else
    ecall_init(g_eid, &ret, args);
#endif
    if (ret) return 2;

    SocketEventLoop communication(iqueue, iqmutex, iqcv);
    for (int i = 0; i < N; ++i) {
        std::thread t(enclave_thread, i, communication);
        t.detach();
    }

    set_logmask(~0);
    std::signal(SIGINT, ctrlc_handler);
    std::signal(SIGPIPE, sigpipe_handler);

    communication.set_listener("*", args.port);
    communication.event_loop();
}

