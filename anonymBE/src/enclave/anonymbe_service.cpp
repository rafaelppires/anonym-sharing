#include <enclave_anonymbe_t.h>
#include <anonymbe_service.h>
#include <sgx_cryptoall.h>
#include <stdio.h>
#include <libc_mock/libcpp_mock.h>

extern int printf(const char*,...);
//------------------------------------------------------------------------------
const std::string AnonymBE::eol    = "\r\n",
                  AnonymBE::posrep = "HTTP/1.1 200 OK" + eol +
                                 "Connection: Keep-Alive" + eol +
                                 "Content-Type: application/json" + eol +
                                 "Content-Length: ",
                  AnonymBE::negrep = "HTTP/1.1 400 Bad Request" + eol +
                                 "Connection: Keep-Alive" + eol +
                                 "Content-Type: application/json" + eol +
                                 "Content-Length: ",
                  AnonymBE::magic  = "\xCA\xFE\x41MCS";

//------------------------------------------------------------------------------
AnonymBE::AnonymBE() : init_(false), error_(AMCS_NOERROR), update_number_(0),
               update_written_(0), die_(false), loaded_file_version_(0) {
    update_mutex_ = SGX_THREAD_MUTEX_INITIALIZER;
    goahead_condition_ = end_condition_ = update_condition_ = SGX_THREAD_COND_INITIALIZER;
    memset( &counter_uuid_, 0, sizeof(counter_uuid_) );
}

//------------------------------------------------------------------------------
bool AnonymBE::check_format( const std::string &verb, const std::string &operation,
                         const std::string &input, std::string &output ) {
    size_t pos;
    std::string urlbeg("/monotonicCounter/");

    if( (pos = input.find(verb)) != std::string::npos ) {
        if( (pos = input.find( urlbeg, pos+verb.size() )) 
                                                         != std::string::npos) {
            pos += urlbeg.size();
            size_t pos1;
            if( (pos1 = input.find("/", pos)) != std::string::npos ) {
                std::string id = input.substr(pos,pos1-pos).c_str();
                if( input.find( operation, pos1+1 ) != std::string::npos ) {
                    output = id;
                    return true;
                }
            }
        }
    }
    return false;
}

//------------------------------------------------------------------------------
void AnonymBE::process_input( std::string &rep, const char *buff, size_t len ) {
    if( !init_ ) init();

    std::string input(buff,len), id;
    bool post, put, get;
    int value = -1;
    AMCSError error = AMCS_NOERROR;

    if( !(post = check_format( "POST", "counter", input, id )) && 
        !(put  = check_format( "PUT",  "inc",     input, id )) && 
        !(get  = check_format( "GET",  "get",     input, id )) )
        error = AMCS_BAD_REQUEST;
    else {
        error = AMCS_NOERROR;
        std::string hash = Crypto::sha256( id );
        if( post ) {
            if( counter_table_.find(hash) != counter_table_.end() )
                error = AMCS_CREATE_EXISTENT;
            else {
                sgx_thread_mutex_lock(&update_mutex_);
                value = counter_table_[hash] = 0;
            }
        } else if( put ) {
            if( counter_table_.find(hash) == counter_table_.end() )
                error = AMCS_NON_EXISTENT;
            else {
                sgx_thread_mutex_lock(&update_mutex_);
                value = ++counter_table_[hash];
            }
        } else if( get ) {
            if( counter_table_.find(hash) == counter_table_.end() )
                error = AMCS_NON_EXISTENT;
            else {
//                sgx_thread_mutex_lock(&update_mutex_);
                value = counter_table_[hash];
//                sgx_thread_mutex_unlock(&update_mutex_);
            }
        }
    }

    if( error == AMCS_NOERROR && (post || put) ) {
        uint64_t thisupdate = ++update_number_;
        // signal update thread 
        sgx_thread_cond_signal( &update_condition_ );
        // wait until file is written OR increment is made
        //while( thisupdate > update_written_ && !die_ )
        //    sgx_thread_cond_wait( &goahead_condition_, &update_mutex_ );
        if( die_ )
            error = AMCS_WRAPUP;
        sgx_thread_mutex_unlock(&update_mutex_);
    }

    if( error == AMCS_NOERROR )
        set_positive_response(rep, value);
    else
        set_negative_response(rep, id, err_amcs(error));

}

//------------------------------------------------------------------------------
void AnonymBE::increment() {
    sgx_status_t ret = 
           sgx_increment_monotonic_counter( &counter_uuid_, &master_counter_ );
    if( ret != SGX_SUCCESS )
        printf("Error incrementing MC: %s\n", err_msg(ret).c_str() );
}

//------------------------------------------------------------------------------
void AnonymBE::set_positive_response( std::string &rep, size_t value ) {
    std::string content = "{\"result\":\"OK\",\"value\":\""
                                                + std::to_string(value) + "\"}";
    rep =  posrep + std::to_string(content.size()) + eol + eol + content + eol;
}

//------------------------------------------------------------------------------
void AnonymBE::set_negative_response( std::string &rep, const std::string &key,
                                                    const std::string &msg ) {
    std::string content = "{\"pkey\":\"" + key + "\",\"msg\":\"" + msg + "\"}";
    rep = negrep + std::to_string(content.size()) + eol + eol + content + eol;
}

//------------------------------------------------------------------------------
int AnonymBE::input_file( const std::string &data ) {
    if( init_ ) {
        printf("Input files must be loaded before initialization\n");
        return -1;
    } else if( error_ == AMCS_ROLLBACK_ATTACK ) {
        error_ = AMCS_NOERROR;
    }

    size_t pos = magic.size() + sizeof(counter_uuid_);
    if( data.size() < magic.size() || data.substr(0,magic.size()) != magic ) {
        printf("Bad magic\n");
        return -3;
    } else if( data.size() >= pos ) {
        std::string uuid = data.substr( magic.size(), sizeof(counter_uuid_) );
        memcpy( &counter_uuid_, uuid.c_str(), uuid.size() );

        if( data.size() >= pos + sizeof(master_counter_) ) {
            uint32_t version;
            std::string v = data.substr(pos, sizeof(master_counter_) );
            pos += sizeof(master_counter_);
            memcpy( &version, v.c_str(), v.size() );
            printf("File version: %u\n", version);
            if( version > loaded_file_version_ ) {
                loaded_file_version_ = version;
                counter_table_.clear();
                while( data.size() - pos >= 36 ) {
                    counter_table_[ data.substr(pos,32) ] = 
                                      *(uint32_t*)data.substr(pos+32,4).c_str();
                    pos += 36;
                }
            }
        }
        return 0;
    }
    return -4;
}

//------------------------------------------------------------------------------
int AnonymBE::init() {
    if( init_ && 
        (error_ == AMCS_ERROR_SERVICE || error_ == AMCS_INITIALIZATION_FAIL) )
        init_ = false;
    else if( error_ == AMCS_ROLLBACK_ATTACK ) {
        printf( "Rollback attack detected. "
                "Cannot init unless the correct file is loaded\n");
        init_ = false;
        return error_;
    } else if( init_ )
        return AMCS_ALREADYINITIALIZED;

    sgx_status_t ret; 

    int busy_retry_times = 2;
    do {
        ret = sgx_create_pse_session();
    } while( ret == SGX_ERROR_BUSY && busy_retry_times-- );

    if( ret != SGX_SUCCESS ) {
        error_ = AMCS_ERROR_SERVICE;
        printf("%s: %s\n", err_amcs(error_).c_str(), err_msg(ret).c_str() );
        return error_;
    } else {
        bool create = false;
        error_ = AMCS_NOERROR;
        sgx_mc_uuid_t dummy;
        memset( &dummy, 0, sizeof(dummy) );
        if( memcmp( &dummy, &counter_uuid_, sizeof(dummy) ) == 0 ) {
            ret = sgx_create_monotonic_counter( &counter_uuid_, 
                                                &master_counter_ );
            create = true;
            printf("Created counter\n");
        } else {
            ret = sgx_read_monotonic_counter(&counter_uuid_, &master_counter_);
        }

        if( ret != SGX_SUCCESS ) {
            error_ = AMCS_INITIALIZATION_FAIL;
            printf("%s counter: %s\n", create ? "create" : "read", 
                                       err_msg(ret).c_str() );
            return error_;
        } else error_ = AMCS_NOERROR;
    }

    sgx_thread_condattr_t unused1;
    printmsg_onerror( sgx_thread_cond_init( &update_condition_, &unused1 ),
                      "Error creating condition var" );

    printmsg_onerror( sgx_thread_cond_init( &goahead_condition_, &unused1 ),
                      "Error creating condition var" );

    printmsg_onerror( sgx_thread_cond_init( &end_condition_, &unused1 ),
                      "Error creating condition var" );

    sgx_thread_mutexattr_t unused2;
    printmsg_onerror( sgx_thread_mutex_init( &update_mutex_, &unused2),
                      "Error creating mutex" );

    printf("MC(%s) = %d\n", Crypto::printable(std::string((const char*)&counter_uuid_, sizeof(counter_uuid_))).c_str(), master_counter_);
    if( 
#if NOROLLBACKCHECK
        false
#else
        loaded_file_version_ < master_counter_ 
#endif
      ) {
#ifdef FAST_UNSAFE
        if( master_counter_ - loaded_file_version_ == 1 ) {
            printf("Versions differ by 1. Possible rollback attack.\n");
            increment();
            init_ = true;
        } else {
#endif
        printf("Rollback attack detected\n");
        error_ = AMCS_ROLLBACK_ATTACK;
        init_ = false;
#ifdef FAST_UNSAFE
        }
#endif
    } else
        init_ = true;
    return (int)error_;
}

//------------------------------------------------------------------------------
bool AnonymBE::printmsg_onerror( int ret, const char *msg ) {
    if( ret != SGX_SUCCESS ) 
        printf("%s: %s\n", msg, err_msg(ret).c_str() );
    return ret == SGX_SUCCESS;
} 

//------------------------------------------------------------------------------
void AnonymBE::finish() {
    // signal other threads to finish
    sgx_thread_mutex_lock(&update_mutex_);
    die_ = true;
    sgx_thread_cond_broadcast( &update_condition_ );
    // wait signal from update thread
    sgx_thread_cond_wait( &end_condition_, &update_mutex_ );
    sgx_thread_mutex_unlock(&update_mutex_);

    printmsg_onerror( sgx_close_pse_session(), "Error closing PS session" );
    printmsg_onerror( sgx_thread_cond_destroy( &update_condition_ ),
                                             "Error destroying condition var" );
    printmsg_onerror( sgx_thread_cond_destroy( &goahead_condition_ ),
                                             "Error destroying condition var" );
    printmsg_onerror( sgx_thread_cond_destroy( &end_condition_ ),
                                             "Error destroying condition var" );
    printmsg_onerror( sgx_thread_mutex_destroy( &update_mutex_ ),
                                            "Error destroying mutex" );

    //wait other threads
}

//------------------------------------------------------------------------------
std::string AnonymBE::err_msg( int v ) {
    switch(v) {
    case SGX_ERROR_AE_SESSION_INVALID:
        return "Session is not created or has been closed "
               "by architectural enclave service";
    case SGX_ERROR_BUSY:
        return "Service is busy: try later";
    default:
        return "Unknown error code (" + std::to_string(v) + ")";
    };
}

//------------------------------------------------------------------------------
std::string AnonymBE::err_amcs( AMCSError e ) {
    switch(e) {
    case AMCS_NOERROR:             return "No error";
    case AMCS_INITIALIZATION_FAIL: return "Fail to initialize AnonymBE";
    case AMCS_ERROR_SERVICE:       return "Could not access Platform Services";
    case AMCS_BAD_REQUEST:         return "Malformed request: "
                                          "refer to the documentation";
    case AMCS_CREATE_EXISTENT:     return "Tried to create an existing counter";
    case AMCS_NON_EXISTENT:        return "Tried to get or update a "
                                          "non-existing counter";
    case AMCS_WRAPUP:              return "Service is being terminated";
    default:                       return "Unknown error";
    }
}

//------------------------------------------------------------------------------

