#include <anonymbe_service.h>
#include <enclave_anonymbe_t.h>
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

std::map<int,std::pair<int,std::string>> html_buff;
std::mutex buff_lock;

bool html_buffer( int fd, std::string &input ) {
    std::lock_guard<std::mutex> lock(buff_lock);

    //printf("<%s>\n", input.c_str());
    size_t pos;
    if( html_buff.find(fd) != html_buff.end() && 
            html_buff[fd].first == input.size() ) {
        input = html_buff[fd].second + input;
        html_buff.erase(fd);
        return false;
    }

    if ((pos=input.find("Content-Length")) != std::string::npos) {
        if((pos=input.find(':',pos)) != std::string::npos) {
            size_t n;
            ++pos;
            int content_length = std::stoi(input.substr(pos),&n);
            if( (pos=input.find("\r\n\r\n",pos+n)) != std::string::npos ) {
                int header_length = pos+4;
                //printf("Input size: %d Content-length: %d Last pos: %d"
                //        " Header length: %d\n", 
                //        input.size(), content_length, pos, header_length);
                if (input.size() >= header_length + content_length) {
                    return false;
                } else {
                    html_buff[fd].first = content_length;
                    html_buff[fd].second += input;
                    return true;
                }
            }
        }
    } else if( html_buff.find(fd) == html_buff.end() ) {
        printf("Not cool\n");
    }
    return true; 
}

//------------------------------------------------------------------------------
int ecall_query(int fd, const char *buff, size_t len) {
    try {
        std::string input(buff,len), response;
        //printf("Received: %d\n",len);
        if( html_buffer(fd, input) ) {
          return 0;
        }

        anonymbe.process_input(input, response);
        ssize_t ret;
#ifdef TLS_REQUESTS
        int r = tls_send(fd, response.c_str(), response.size());
#else
        ocall_send(&ret, fd, response.c_str(), response.size(), 0);
#endif
    } catch(...) {
        printf(":_(   o.O   ;_;\n");
    }
    return 0;
}

//------------------------------------------------------------------------------
int ecall_init(struct Arguments args) {
    return anonymbe.init(&args);
}

//------------------------------------------------------------------------------
int ecall_tls_accept(int fd) { return anonymbe.accept(fd);}

//------------------------------------------------------------------------------
void ecall_finish() { anonymbe.finish(); }

