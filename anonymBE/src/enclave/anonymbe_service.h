#ifndef _AMCS_SERVICE_H_
#define _AMCS_SERVICE_H_

#include <string>
#include <sgx_tae_service.h>
#include <sgx_thread.h>
#include <map>
typedef std::map<std::string,std::string> KVString;

template< typename Database >
class AnonymBE {
public:
    AnonymBE();
    void process_input( std::string &rep, const char *buff, size_t len );
    void set_positive_response( std::string &rep,
                                const KVString &response );
    void set_negative_response( std::string &rep, const std::string &msg,
                                                  const std::string &detail );
    int input_file( const std::string &data );

    int init();
    void finish();

private:
    enum AMCSError {
        AMCS_NOERROR,
        AMCS_ERROR_SERVICE,
        AMCS_INITIALIZATION_FAIL,
        AMCS_ALREADYINITIALIZED,
        AMCS_BAD_REQUEST,
        AMCS_CREATE_EXISTENT,
        AMCS_NON_EXISTENT,
        AMCS_ROLLBACK_ATTACK,
        AMCS_WRAPUP,
        AMCS_UNKNOWN
    } error_;

    AMCSError process_get   ( const std::string &command, 
                              const std::string &content, KVString &rep );
    AMCSError process_put   ( const std::string &command, 
                              const std::string &content );
    AMCSError process_post  ( const std::string &command, 
                              const std::string &content, KVString &rep );
    AMCSError process_delete( const std::string &command, 
                              const std::string &content );

    std::string mongo_error( uint32_t );
    std::string err_msg( int );
    std::string err_amcs( AMCSError e );
    bool http_parse( const std::string &input, std::string &verb,
                           std::string &command, std::string &content );
    bool printmsg_onerror( int ret, const char *msg );

    static const std::string eol,posrep,negrep,magic;

    // internal state
    bool init_, die_;

    // database
    Database database_;

    // synchronization stuff
    sgx_thread_cond_t update_condition_, goahead_condition_, end_condition_;
    sgx_thread_mutex_t update_mutex_;
};

extern "C" { int printf(const char *fmt, ...); }

#include <anonymbe_service.hpp>

#endif

