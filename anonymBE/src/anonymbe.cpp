#define N 9 // input/enclave service threads

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <iostream>
#include <queue>
#include <thread>
#include <csignal>
#include <fstream>
#include <streambuf>

#include <sgx_initenclave.h>
#include <sgx_utils_rp.h>

#include <event_loop.h>
#include <enclave_anonymbe_u.h>

sgx_enclave_id_t g_eid;

std::condition_variable iqcv,oqcv;
std::mutex iqmutex, oqmutex;

FdQueue iqueue; // ready file descriptors
DataQueue oqueue;

//------------------------------------------------------------------------------
struct Arguments {
    size_t port;
};

//------------------------------------------------------------------------------
void enclave_thread( int id, const SocketEventLoop &comm ) {
    for(;;) {
        int fd = -1;
        { // sleeps until there is data
            std::unique_lock<std::mutex> lk(iqmutex);
            iqcv.wait( lk, []{ return !iqueue.empty(); } );
            fd = iqueue.front();
            iqueue.pop();
        }

        std::string msg;
        comm.consume_fd( fd, msg ); 

        size_t msglen = msg.size(), 
               eoflen = SocketEventLoop::eof.size();
        if( msglen >= eoflen && msg.substr( msglen-eoflen, eoflen ) 
                                                     == SocketEventLoop::eof ) {
            //printf("Connection closed\n");
        } else {
            int ret;
            ecall_query( g_eid, &ret, fd, msg.c_str(), msg.size() );
        }
    }
}

//------------------------------------------------------------------------------
void ctrlc_handler( int s ) {
    printf("\033[0m\n(%d) bye!\n", s);
    ecall_finish( g_eid );
    destroy_enclave( g_eid );
    exit(0);
}

//------------------------------------------------------------------------------
int main( int argc, char** argv ) {
    /* Changing dir to where the executable is.*/
    change_dir( argv[0] );

     // Sets up enclave and initializes remote_attestation
    if(initialize_enclave(g_eid,"enclave_anonymbe.signed.so","enclave.anonymbe.token"))
        return 1;

    int ret;
    ecall_init( g_eid, &ret );
    if( ret )
        return 2; 

    SocketEventLoop communication( iqueue, iqmutex, iqcv );
    for( int i = 0; i < N; ++i ) {
        std::thread t( enclave_thread, i, communication );
        t.detach();
    }

    set_logmask( ~0 );
    std::signal( SIGINT, ctrlc_handler );

    Arguments args;
    args.port = 4444;

    communication.set_listener( "*", args.port );
    communication.event_loop();
}

