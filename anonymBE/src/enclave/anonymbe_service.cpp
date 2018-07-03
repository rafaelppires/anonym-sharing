#include <enclave_anonymbe_t.h>
#include <anonymbe_service.h>
#include <sgx_cryptoall.h>
#include <stdio.h>
#include <libc_mock/libcpp_mock.h>
#include <json/json.hpp>

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
bool AnonymBE::http_parse( const std::string &input, std::string &verb,
                           std::string &command, std::string &content ) {
    size_t pos;
    if( input.find("HTTP/") != std::string::npos ) {
       if( (pos=input.find(' ')) != std::string::npos ) {
            verb = input.substr(0,pos);
            size_t pos1; ++pos;
            if( (pos1=input.find(' ', pos)) != std::string::npos ) {
                command = input.substr(pos,pos1-pos);
                ++pos1;
                if( (pos=input.find("\n\n")) != std::string::npos ) {
                    content = input.substr( pos+2 );
                    return true;
                } else if( (pos=input.find("\r\n\r\n")) != std::string::npos ) {
                    content = input.substr( pos+4 );
                    return true;
                }
            }
       }
    }
    return false;
}

//------------------------------------------------------------------------------
using json = nlohmann::json;
//------------------------------------------------------------------------------
AnonymBE::AMCSError AnonymBE::process_get( const std::string &command, 
                            const std::string &content ) {
    if       ( command == "/access/rights" ) {
    } else if( command == "/verifier/certify" ) {
    } else if( command == "/verifier/envelope" ) {
    }
    return AMCS_BAD_REQUEST;
}

//------------------------------------------------------------------------------
AnonymBE::AMCSError AnonymBE::process_put( const std::string &command, 
                            const std::string &content ) {
    if       ( command == "/access/usergroup" ) {
    } else if( command == "/access/aclmember" ) {
    } else if( command == "/access/bucketowner" ) {
    }
    return AMCS_BAD_REQUEST;
}

//------------------------------------------------------------------------------
AnonymBE::AMCSError AnonymBE::process_post( const std::string &command, 
                                            const std::string &content ) {
    if       ( command == "/access/user" ) {
    } else if( command == "/access/group" ) {
        auto j = json::parse(content);
        printf("%s\n", j.dump(4).c_str());
    } else if( command == "/access/acl" ) {
    } else if( command == "/verifier/acl" ) {
    } else if( command == "/writer/bucket" ) {
    }
    return AMCS_BAD_REQUEST;
}

//------------------------------------------------------------------------------
AnonymBE::AMCSError AnonymBE::process_delete( const std::string &command, 
                               const std::string &content ) {
    if       ( command == "/access/usergroup" ) {
    } else if( command == "/access/aclmember" ) {
    }
    return AMCS_BAD_REQUEST;
}

//------------------------------------------------------------------------------
void AnonymBE::process_input( std::string &rep, const char *buff, size_t len ) {
    if( !init_ ) init();

    std::string input(buff,len), id, verb, command, content;
    bool post, put, get;
    AMCSError error = AMCS_NOERROR;

    if( http_parse( input, verb, command, content ) ) {
        if       ( verb == "GET" ) {
            error = process_get( command, content ); 
        } else if( verb == "PUT" ) {    
            error = process_put( command, content );
        } else if( verb == "POST" ) {   
            error = process_post( command, content );
        } else if( verb == "DELETE" ) { 
            error = process_delete( command, content );
        }
    } else {
        error = AMCS_BAD_REQUEST;
    }

    if( error == AMCS_NOERROR )
        set_positive_response(rep);
    else
        set_negative_response(rep, id, err_amcs(error));

}

//------------------------------------------------------------------------------
void AnonymBE::increment() {
}

//------------------------------------------------------------------------------
void AnonymBE::set_positive_response( std::string &rep ) {
    std::string content = "{\"result\":\"OK\"}";
    rep =  posrep + std::to_string(content.size()) + eol + eol + content + eol;
}

//------------------------------------------------------------------------------
void AnonymBE::set_negative_response( std::string &rep, const std::string &key,
                                                    const std::string &msg ) {
    std::string content = "{\"result\":\"error\",\"msg\":\"" + msg + "\"}";
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

