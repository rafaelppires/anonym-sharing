#ifndef _AMCS_SERVICE_H_
#define _AMCS_SERVICE_H_

#include <string>
#include <sgx_tae_service.h>
#include <map>
#include <sgx_thread.h>

class AnonymBE {
public:
    AnonymBE();
    void process_input( std::string &rep, const char *buff, size_t len );
    void set_positive_response( std::string &rep );
    void set_negative_response( std::string &rep, const std::string &key,
                                                  const std::string &msg );
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
        AMCS_WRAPUP
    } error_;

    void process_get( const std::string &command, const std::string &content );
    void process_put( const std::string &command, const std::string &content );
    void process_post( const std::string &command, const std::string &content );
    void process_delete( const std::string &command, 
                         const std::string &content );

    std::string err_msg( int );
    std::string err_amcs( AMCSError e );
    void increment();
    bool http_parse( const std::string &input, std::string &verb,
                           std::string &command, std::string &content );
    bool printmsg_onerror( int ret, const char *msg );

    static const std::string eol,posrep,negrep,magic;

    // file control
    uint32_t loaded_file_version_;

    // internal state
    std::map< std::string, uint32_t > counter_table_;
    sgx_mc_uuid_t counter_uuid_;
    uint32_t master_counter_;
    uint64_t update_number_, update_written_;
    bool init_, die_;

    // synchronization stuff
    sgx_thread_cond_t update_condition_, goahead_condition_, end_condition_;
    sgx_thread_mutex_t update_mutex_;
};

#endif

