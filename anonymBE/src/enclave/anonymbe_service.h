#ifndef _ASKY_SERVICE_H_
#define _ASKY_SERVICE_H_

#include <sgx_tae_service.h>
#include <sgx_thread.h>
#include <map>
#include <string>
#include <anonymbe_args.h>
#ifdef TLS_REQUESTS
#include <tls_config.h>
#endif
typedef std::map<std::string, std::string> KVString;

template <typename Database>
class AnonymBE {
   public:
    AnonymBE();
    void process_input(const std::string &input, std::string &rep);
    void set_positive_response(std::string &rep, const KVString &response);
    void set_negative_response(std::string &rep, const std::string &msg,
                               const std::string &detail);
    int input_file(const std::string &data);

    int init(Arguments *);
    int accept(int fd);
    void finish();

   private:
    enum ASKYError {
        ASKY_NOERROR,
        ASKY_ERROR_SERVICE,
        ASKY_INITIALIZATION_FAIL,
        ASKY_ALREADYINITIALIZED,
        ASKY_BAD_REQUEST,
        ASKY_CREATE_EXISTENT,
        ASKY_NON_EXISTENT,
        ASKY_ROLLBACK_ATTACK,
        ASKY_WRAPUP,
        ASKY_UNKNOWN
    } error_;

    ASKYError process_get(const std::string &command,
                          const std::string &content, KVString &rep);
    ASKYError process_put(const std::string &command,
                          const std::string &content);
    ASKYError process_post(const std::string &command,
                           const std::string &content, KVString &rep);
    ASKYError process_delete(const std::string &command,
                             const std::string &content);

    std::string mongo_error(uint32_t);
    std::string err_msg(int);
    std::string err_amcs(ASKYError e);
    bool http_parse(const std::string &input, std::string &verb,
                    std::string &command, std::string &content);
    bool printmsg_onerror(int ret, const char *msg);

    static const std::string eol, posrep, negrep, magic;

    // internal state
    bool init_, die_;
#ifdef TLS_REQUESTS
    SSL_CTX *ctx_;
#endif

    // database
    Database database_;

    // synchronization stuff
    sgx_thread_cond_t update_condition_, goahead_condition_, end_condition_;
    sgx_thread_mutex_t update_mutex_;
};

extern "C" {
int printf(const char *fmt, ...);
}

#include <anonymbe_service.hpp>

#endif
