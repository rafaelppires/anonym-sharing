#include <event_loop.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sgx_utils_rp.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

const std::string SocketEventLoop::eof = "\033EOF\033";
//------------------------------------------------------------------------------
void SocketEventLoop::set_listener( const std::string &host,
                                    unsigned short port ) {
    listendpoint_ = host + ":" + std::to_string(port);
    bzero(&localserver_, sizeof(localserver_));
    localserver_.sin_addr.s_addr =  host == "*" ? htonl(INADDR_ANY) 
                                           : inet_addr( host.c_str() );
    localserver_.sin_port = htons( port );
    localserver_.sin_family = AF_INET;
}

//------------------------------------------------------------------------------
void SocketEventLoop::bind_n_listen( int &fd ) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, 4);
    if( bind(fd, 
        (struct sockaddr *) &localserver_, sizeof(localserver_)) < 0 ) {
        exit_error(1, "Error binding to %s\n", listendpoint_.c_str());
    } else {
        printinfo( LLINFO, "Listening %s\n", listendpoint_.c_str());
    }

    listen(fd, 10);
}

//------------------------------------------------------------------------------
void SocketEventLoop::set_server( const std::string &host,
                                  unsigned short port ) {
    connectendpoint_ = host + ":" + std::to_string(port);
    bzero(&remoteserver_, sizeof(remoteserver_));
    remoteserver_.sin_addr.s_addr = inet_addr( host.c_str() );
    remoteserver_.sin_port = htons( port );
    remoteserver_.sin_family = AF_INET;
}

//------------------------------------------------------------------------------
void SocketEventLoop::connect( int &fd ) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if( ::connect(fd, (struct sockaddr*)&remoteserver_, sizeof(remoteserver_)) 
                                                                           < 0 )
        printinfo( LLWARN, "Unable to connect to server %s\n", 
                           connectendpoint_.c_str() );
}

//------------------------------------------------------------------------------
void SocketEventLoop::consume_fd( int fd, std::string &msg ) const {
    ssize_t sz;
    char buff[1000];
   
    while( (sz = recv( fd, buff, sizeof(buff), 0 )) >= 0 ) {
        if( sz == 0 ) {
            msg += eof;
            close(fd);
            break;
        } else {
            msg += std::string( buff, sz );
        }
    }

    if( sz < 0 ) {
        if( errno == EAGAIN || errno == EBADF ) {
        } else if( errno == ECONNRESET ) {
            close( fd );
        } else
            printinfo( LLWARN, "Unexpected error (%d): %s\n", 
                                                       errno, strerror(errno) );
    }
}

//------------------------------------------------------------------------------
void SocketEventLoop::setnonblocking(int sock) {
    int opts;
    opts = fcntl(sock, F_GETFL);
    if(opts < 0) exit_error( 1, "fcntl(sock, GETFL)");

    opts = opts | O_NONBLOCK;
    if(fcntl(sock, F_SETFL, opts) < 0)
        exit_error( 1, "fcntl(sock, SETFL, opts)");
}

//------------------------------------------------------------------------------
void SocketEventLoop::create_epoll() {
    epollfd_ = epoll_create1(0);
    if (epollfd_ == -1)
        exit_error( EXIT_FAILURE, "epoll_create1" );
}

//------------------------------------------------------------------------------
void SocketEventLoop::addto_epoll( int fd, uint32_t events ) {
    //static unsigned int n = 0;printf("%u\n",++n);
    struct epoll_event ev;
    if( events & EPOLLET ) setnonblocking(fd);
    ev.events = events;
    ev.data.fd = fd;
    if( epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev ) == -1)
        exit_error( EXIT_FAILURE, "Error on adding fd %d to epoll\n", fd);
}

//------------------------------------------------------------------------------
#define MAX_EVENTS 10
void SocketEventLoop::event_loop() {
    struct sockaddr_in client;

    if( epollfd_ < 0 ) create_epoll();

    int listen_sock = -1;
    struct epoll_event ev, events[MAX_EVENTS];
    if( !listendpoint_.empty() ) {
        bind_n_listen( listen_sock );
        addto_epoll( listen_sock, EPOLLIN );
    }

    socklen_t client_size = sizeof(client);
    int sock_fd;
    int conn_sock, nfds;

    for(;;) {
        nfds = epoll_wait(epollfd_, events, MAX_EVENTS, -1);
        if (nfds == -1)
            exit_error( EXIT_FAILURE, "epoll_wait" );

        for( int n = 0; n < nfds; ++n ) {
            if (events[n].data.fd == listen_sock) {
                conn_sock = accept( listen_sock,
                                    (struct sockaddr *) &client, &client_size );
                if (conn_sock == -1) {
                    //if( errno == EMFILE ) continue;
                    exit_error( EXIT_FAILURE, "Error in accept: %s\n", strerror(errno) );
                }

                addto_epoll( conn_sock, EPOLLIN | EPOLLET );
            } else {
                //consume_fd( events[n].data.fd );
                int fd = events[n].data.fd;
                {
                    std::lock_guard<std::mutex> lock(iqmutex_);
                    iqueue_.push( fd );
                }
                qcv_.notify_one();
            }
        }
    }
}

