#ifndef _ASKY_SERVICE_H_
#define _ASKY_SERVICE_H_

#include <httprequest.h>
#include <httpresponse.h>
#include <anonymbe_args.h>
#ifndef NATIVE
#include <sgx_tae_service.h>
#include <sgx_thread.h>
#endif
#include <map>
#include <string>
#ifdef TLS_REQUESTS
#include <tls_config.h>
#endif
#ifdef ABEMONGO
#include <bson.h>
#endif
#include <json/json.hpp>
using json = nlohmann::json;
typedef std::map<std::string, std::string> KVString;

template <typename Database>
class AnonymBE {
   public:
    AnonymBE();
    Response process_input(const Request &req);
    Response positive_response(const KVString &response);
    Response negative_response(const std::string &msg,
                               const std::string &detail);
    int input_file(const std::string &data);

    int init(Arguments *);
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

    ASKYError process_get(const Request &, KVString &rep);
    ASKYError process_put(const Request &);
    ASKYError process_post(const Request &, KVString &rep);
    ASKYError process_delete(const Request &);

    ASKYError generate_envelope(json &,KVString &response);
    ASKYError create_user(json &,KVString &response);

#ifdef ABEMONGO
    std::string mongo_error(bson_error_t &);
#endif
    std::string sgx_errmsg(int);
    std::string err_amcs(ASKYError e);
    bool printmsg_onerror(int ret, const char *msg);
    static const Response posrep, negrep;

    // internal state
    bool init_, indexed_, die_;

    // database
    Database database_;

#ifndef NATIVE
    // synchronization stuff
    sgx_thread_cond_t update_condition_, goahead_condition_, end_condition_;
    sgx_thread_mutex_t update_mutex_;
#endif
};

extern "C" {
int printf(const char *fmt, ...);
}

#include <anonymbe_service.hpp>

#endif
