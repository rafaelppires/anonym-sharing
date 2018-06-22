#include <enclave_anonymbe_t.h>
#include <anonymbe_service.h>
#include <sgx_cryptoall.h>
#include <stdio.h>
#include <libc_mock/libcpp_mock.h>
#include <json/json.hpp>

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
}

//------------------------------------------------------------------------------
int AnonymBE::init() {
}

//------------------------------------------------------------------------------
void AnonymBE::finish() {
}

//------------------------------------------------------------------------------
bool AnonymBE::printmsg_onerror( int ret, const char *msg ) {
    if( ret != SGX_SUCCESS ) 
        printf("%s: %s\n", msg, err_msg(ret).c_str() );
    return ret == SGX_SUCCESS;
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

