#include <enclave_anonymbe_t.h>
#include <anonymbe_service.h>
#include <sgx_cryptoall.h>
#include <stdio.h>
#include <libc_mock/libcpp_mock.h>
#include <json/json.hpp>
#include <sgx_trts.h>
#include <database.h>

#define KEY_SIZE 32

#define json_str(var,field) ((var).at(#field).get<std::string>())
//------------------------------------------------------------------------------
template< typename T >
const std::string AnonymBE<T>::eol    = "\r\n";
template< typename T >
const std::string AnonymBE<T>::posrep = "HTTP/1.1 200 OK" + eol +
                                 "Connection: Keep-Alive" + eol +
                                 "Content-Type: application/json" + eol +
                                 "Content-Length: ";
template< typename T >
const std::string AnonymBE<T>::negrep = "HTTP/1.1 400 Bad Request" + eol +
                                 "Connection: Keep-Alive" + eol +
                                 "Content-Type: application/json" + eol +
                                 "Content-Length: ";
template< typename T >
const std::string AnonymBE<T>::magic  = "\xCA\xFE\x41MCS";

//------------------------------------------------------------------------------
template< typename T >
AnonymBE<T>::AnonymBE() : init_(false), error_(AMCS_NOERROR), die_(false) {
    update_mutex_ = SGX_THREAD_MUTEX_INITIALIZER;
    goahead_condition_ = end_condition_ = update_condition_ = SGX_THREAD_COND_INITIALIZER;
}

//------------------------------------------------------------------------------
template< typename T >
bool AnonymBE<T>::http_parse( const std::string &input, std::string &verb,
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
template< typename T >
typename AnonymBE<T>::AMCSError AnonymBE<T>::process_get( 
                      const std::string &command, const std::string &content,
                                                  KVString &response ) {
    if       ( command == "/access/rights" ) {
    } else if( command == "/verifier/certify" ) {
    } else if( command == "/verifier/envelope" ) {
        auto j = json::parse(content);
        std::string bucket_key = j.at("bucket_key").get<std::string>();
        KeyArray ka = database_.get_keys_of_group(
                                          j.at("bucket_id").get<std::string>());
        std::string ctext;
        for( const auto &k : ka ) {
            ctext += Crypto::encrypt_aes( 
                            std::string( (const char*)k.data(), KEY_SIZE ),
                            bucket_key );
        }

        response[ "ciphertext" ] = Crypto::b64_encode(ctext);
        return AMCS_NOERROR;
    }
    return AMCS_BAD_REQUEST;
}

//------------------------------------------------------------------------------
template< typename T >
typename AnonymBE<T>::AMCSError AnonymBE<T>::process_put( const std::string &command, 
                                           const std::string &content ) {
    if       ( command == "/access/usergroup" ) {
        auto j = json::parse(content);
        database_.add_user_to_group( json_str( j, group_name ),
                                     json_str( j, new_member_id ) );
        return AMCS_NOERROR;
    } else if( command == "/access/aclmember" ) {
    } else if( command == "/access/bucketowner" ) {
    }
    return AMCS_BAD_REQUEST;
}

//------------------------------------------------------------------------------
template< typename T >
typename AnonymBE<T>::AMCSError AnonymBE<T>::process_post( 
                         const std::string &command, const std::string &content,
                                                     KVString &response ) {
    if       ( command == "/access/user" ) {
        auto j = json::parse(content);
        unsigned char rnd[ KEY_SIZE ];
        sgx_read_rand( rnd, sizeof(rnd) );
        std::string key = std::string( (const char*)rnd, sizeof(rnd) );
        database_.create_user( json_str( j, user_id ), key );
        //if( key == "" ) return AMCS_CREATE_EXISTENT;
        response[ "user_key" ] = Crypto::b64_encode(key);
        return AMCS_NOERROR;
    } else if( command == "/access/group" ) {
        auto j = json::parse(content);
        database_.create_group( json_str( j,group_name ), 
                                json_str( j,user_id ) );
        return AMCS_NOERROR;
    } else if( command == "/access/acl" ) {
    } else if( command == "/verifier/acl" ) {
    } else if( command == "/writer/bucket" ) {
    }
    return AMCS_BAD_REQUEST;
}

//------------------------------------------------------------------------------
template< typename T >
typename AnonymBE<T>::AMCSError AnonymBE<T>::process_delete( const std::string &command, 
                                              const std::string &content ) {
    if       ( command == "/access/usergroup" ) {
        auto j = json::parse(content);
        database_.remove_user_from_group( json_str( j, group_name ),
                                          json_str( j, revoke_member_id ) );
        return AMCS_NOERROR;
    } else if( command == "/access/aclmember" ) {
    }
    return AMCS_BAD_REQUEST;
}

//------------------------------------------------------------------------------
template< typename T >
void AnonymBE<T>::process_input( std::string &rep, const char *buff, size_t len ) {
    if( !init_ ) init();

    std::string input(buff,len), verb, command, content, extra;
    bool post, put, get;
    AMCSError error = AMCS_NOERROR;

    KVString response;
    if( http_parse( input, verb, command, content ) ) {
        try {
            if       ( verb == "GET" ) {
                error = process_get( command, content, response ); 
            } else if( verb == "PUT" ) {    
                error = process_put( command, content );
            } else if( verb == "POST" ) {   
                error = process_post( command, content, response );
            } else if( verb == "DELETE" ) { 
                error = process_delete( command, content );
            }

            if( error == AMCS_BAD_REQUEST ) {
                extra = "Unknown command '" + command + "'";
            }
        } catch( nlohmann::detail::out_of_range &e ) {
            extra = e.what();
            error = AMCS_BAD_REQUEST;
        } catch( std::invalid_argument &e ) {
            extra = e.what();
            error = AMCS_BAD_REQUEST;
        } catch( std::logic_error &e ) { //warning, not error
            response["info"] = e.what();
        }
    } else {
        extra = "Error parsing HTML";
        error = AMCS_BAD_REQUEST;
    }

    if( error == AMCS_NOERROR )
        set_positive_response(rep, response);
    else
        set_negative_response(rep, err_amcs(error), extra);

}

//------------------------------------------------------------------------------
template< typename T >
void AnonymBE<T>::set_positive_response( std::string &rep, const KVString &response ) {
    json j;
    j["result"] = "OK";
    for( auto &kv : response ) {
        j[kv.first] = kv.second;
    }
    std::string content = j.dump(2);
    rep =  posrep + std::to_string(content.size()) + eol + eol + content + eol;
}

//------------------------------------------------------------------------------
template< typename T >
void AnonymBE<T>::set_negative_response( std::string &rep, const std::string &msg,
                                                    const std::string &extra ) {
    json j;
    j["result"] = "error";
    j["msg"] = msg;
    if( extra != "" )
        j["detail"] = extra;
    std::string content = j.dump(2);
    rep = negrep + std::to_string(content.size()) + eol + eol + content + eol;
}

//------------------------------------------------------------------------------
template< typename T >
int AnonymBE<T>::input_file( const std::string &data ) {
}

//------------------------------------------------------------------------------
template< typename T >
int AnonymBE<T>::init() {
}

//------------------------------------------------------------------------------
template< typename T >
void AnonymBE<T>::finish() {
}

//------------------------------------------------------------------------------
template< typename T >
bool AnonymBE<T>::printmsg_onerror( int ret, const char *msg ) {
    if( ret != SGX_SUCCESS ) 
        printf("%s: %s\n", msg, err_msg(ret) );
    return ret == SGX_SUCCESS;
} 

//------------------------------------------------------------------------------
template< typename T >
std::string AnonymBE<T>::err_msg( int v ) {
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
template< typename T >
std::string AnonymBE<T>::err_amcs( AMCSError e ) {
    switch(e) {
    case AMCS_NOERROR:             return "No error";
    case AMCS_INITIALIZATION_FAIL: return "Fail to initialize AnonymBE";
    case AMCS_ERROR_SERVICE:       return "Could not access Platform Services";
    case AMCS_BAD_REQUEST:         return "Malformed request. "
                                          "Please, refer to the documentation";
    case AMCS_CREATE_EXISTENT:     return "Tried to create an existing user";
    case AMCS_NON_EXISTENT:        return "Tried to get or update a "
                                          "non-existing counter";
    case AMCS_WRAPUP:              return "Service is being terminated";
    default:                       return "Unknown error";
    }
}

//------------------------------------------------------------------------------

