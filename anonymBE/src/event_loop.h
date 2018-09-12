#ifndef _SOCKET_EVENTLOOP_H_
#define _SOCKET_EVENTLOOP_H_

#include <netinet/in.h>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>

typedef std::pair<int, std::string> QueueItem;
typedef std::queue<QueueItem> DataQueue;
typedef std::queue<int> FdQueue;

class SocketEventLoop {
   public:
    SocketEventLoop(FdQueue &q, std::mutex &m, std::condition_variable &cv)
        : iqueue_(q), iqmutex_(m), qcv_(cv), epollfd_(-1) {}

    void set_listener(const std::string &host, unsigned short port);
    void set_server(const std::string &host, unsigned short port);
    void event_loop();
    void consume_fd(int fd, std::string &msg_out) const;
    void connect(int &fd);
    void create_epoll();
    void addto_epoll(int fd, uint32_t events);

    static const std::string eof;

   private:
    int epollfd_;
    void setnonblocking(int sock);
    void bind_n_listen(int &fd);
    struct sockaddr_in localserver_, remoteserver_;
    std::string listendpoint_, connectendpoint_;

    FdQueue &iqueue_;
    std::mutex &iqmutex_;
    std::condition_variable &qcv_;
};

#endif
