#include <anonymbe_service.h>
#include <database.h>
#include <enclave_anonymbe_t.h>
#include <libc_mock/libcpp_mock.h>
#include <sgx_cryptoall.h>
#include <sgx_trts.h>
#include <stdio.h>
#include <json/json.hpp>

#define KEY_SIZE 32

#define json_str(var, field) ((var).at(field).get<std::string>())
//------------------------------------------------------------------------------
template <typename T>
const std::string AnonymBE<T>::eol = "\r\n";
template <typename T>
const std::string AnonymBE<T>::posrep =
    "HTTP/1.1 200 OK" + eol + "Connection: Keep-Alive" + eol +
    "Content-Type: application/json" + eol + "Content-Length: ";
template <typename T>
const std::string AnonymBE<T>::negrep =
    "HTTP/1.1 400 Bad Request" + eol + "Connection: Keep-Alive" + eol +
    "Content-Type: application/json" + eol + "Content-Length: ";
template <typename T>
const std::string AnonymBE<T>::magic = "\xCA\xFE\x41MCS";

//------------------------------------------------------------------------------
template <typename T>
AnonymBE<T>::AnonymBE()
    : /*database_(true),*/ init_(false), error_(ASKY_NOERROR), die_(false) {
    update_mutex_ = SGX_THREAD_MUTEX_INITIALIZER;
    goahead_condition_ = end_condition_ = update_condition_ =
        SGX_THREAD_COND_INITIALIZER;
}

//------------------------------------------------------------------------------
template <typename T>
bool AnonymBE<T>::http_parse(const std::string &input, std::string &verb,
                             std::string &command, std::string &content) {
    size_t pos;
    if (input.find("HTTP/") != std::string::npos) {
        if ((pos = input.find(' ')) != std::string::npos) {
            verb = input.substr(0, pos);
            size_t pos1;
            ++pos;
            if ((pos1 = input.find(' ', pos)) != std::string::npos) {
                command = input.substr(pos, pos1 - pos);
                ++pos1;
                if ((pos = input.find("\n\n")) != std::string::npos) {
                    content = input.substr(pos + 2);
                    return true;
                } else if ((pos = input.find("\r\n\r\n")) !=
                           std::string::npos) {
                    content = input.substr(pos + 4);
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
template <typename T>
typename AnonymBE<T>::ASKYError AnonymBE<T>::process_get(
    const std::string &command, const std::string &content,
    KVString &response) {
    if (command == "/access/rights") {
    } else if (command == "/verifier/certify") {
    
    } else if (command == "/ping") {
        // GET endpoint for Kubernetes probe
        return ASKY_NOERROR;
    }

    return ASKY_BAD_REQUEST;
}

//------------------------------------------------------------------------------
template <typename T>
typename AnonymBE<T>::ASKYError AnonymBE<T>::process_put(
    const std::string &command, const std::string &content) {
    if (command == "/access/usergroup") {
        auto j = json::parse(content);
        database_.add_user_to_group(json_str(j, "group_name"),
                                    json_str(j, "user_id"));
        return ASKY_NOERROR;
    } else if (command == "/access/aclmember") {
    } else if (command == "/access/bucketowner") {
    }
    return ASKY_BAD_REQUEST;
}

//------------------------------------------------------------------------------
template <typename T>
typename AnonymBE<T>::ASKYError AnonymBE<T>::process_post(
   const std::string &command, const std::string &content, KVString &response) {
    auto j = json::parse(content);
    if (command == "/access/user") {
        unsigned char rnd[KEY_SIZE];
        sgx_read_rand(rnd, sizeof(rnd));
        std::string key = std::string((const char *)rnd, sizeof(rnd));
        database_.create_user(json_str(j, "user_id"), key);
        // if( key == "" ) return ASKY_CREATE_EXISTENT;
        response["user_key"] = Crypto::b64_encode(key);
        return ASKY_NOERROR;
    } else if (command == "/access/group") {
        database_.create_group(json_str(j, "group_name"),
                               json_str(j, "user_id"));
        return ASKY_NOERROR;
    } else if (command == "/verifier/envelope") {
        std::string ctext;
        std::string bucket_key = Crypto::b64_decode(json_str(j,"bucket_key"));
        bucket_key.resize(32,0);
        KeyArray ka =
            database_.get_keys_of_group(json_str(j,"group_id"));
        for (const auto &k : ka) {
            std::string key((const char *)k.data(), KEY_SIZE);
            ctext += Crypto::encrypt_aesgcm(key, bucket_key);
        }
        response["ciphertext"] = Crypto::b64_encode(ctext);
        return ASKY_NOERROR;
    } else if (command == "/verifier/usergroup") {
        bool answer = database_.is_user_part_of_group( json_str(j, "user_id"), 
                                                    json_str(j, "group_name"));
        response["belongs"] = answer ? "true" : "false";
        return ASKY_NOERROR;
    } else if (command == "/access/acl") {
    } else if (command == "/verifier/acl") {
    } else if (command == "/writer/bucket") {
    }
    return ASKY_BAD_REQUEST;
}

//------------------------------------------------------------------------------
template <typename T>
typename AnonymBE<T>::ASKYError AnonymBE<T>::process_delete(
    const std::string &command, const std::string &content) {
    if (command == "/access/usergroup") {
        auto j = json::parse(content);
        database_.remove_user_from_group(json_str(j, "group_name"),
                                         json_str(j, "user_id"));
        return ASKY_NOERROR;
    } else if (command == "/access/aclmember") {
    } else if (command == "/access/all") {
        database_.delete_all_data();
        return ASKY_NOERROR;
    }
    return ASKY_BAD_REQUEST;
}

//------------------------------------------------------------------------------
template <typename T>
void AnonymBE<T>::process_input( const std::string &input, std::string &rep ) {
    if (!init_) {
#ifdef MEMDATABASE
        init_ = true;
#else
        printf("Attempt to do some processing before initializing\n"); 
        return;
#endif
    } 

    std::string verb, command, content, extra;
    bool post, put, get;
    ASKYError error = ASKY_NOERROR;
    KVString response;
    if (http_parse(input, verb, command, content)) {
        try {
            if (verb == "GET") {
                error = process_get(command, content, response);
            } else if (verb == "PUT") {
                error = process_put(command, content);
            } else if (verb == "POST") {
                error = process_post(command, content, response);
            } else if (verb == "DELETE") {
                error = process_delete(command, content);
            }

            if (error == ASKY_BAD_REQUEST) {
                extra = "Unknown command '" + command + "'";
            }
        } catch (nlohmann::detail::out_of_range &e) {
            extra = e.what();
            error = ASKY_BAD_REQUEST;
        } catch (std::invalid_argument &e) {
            extra = e.what();
            error = ASKY_BAD_REQUEST;
        } catch (std::logic_error &e) {  // warning, not error
            response["info"] = e.what();
        } catch (uint32_t e) {
            extra = mongo_error(e);
            error = ASKY_UNKNOWN;
        } catch (...) {
            error = ASKY_UNKNOWN;
        }
    } else {
        extra = "Error parsing HTML";
        error = ASKY_BAD_REQUEST;
    }

    try {
        if (error == ASKY_NOERROR) {
            set_positive_response(rep, response);
        } else {
            set_negative_response(rep, err_amcs(error), extra);
        }
    } catch(nlohmann::detail::exception &e) {
        printf("Err: %s\n", e.what());
    }
}
//------------------------------------------------------------------------------
template <typename T>
std::string AnonymBE<T>::mongo_error(uint32_t e) {
    switch (e) {
        case 11000:
            return "Attempt to add an existing user";
        default:
            return "Unknown mongo error";
    };
}

//------------------------------------------------------------------------------
template <typename T>
void AnonymBE<T>::set_positive_response(std::string &rep,
                                        const KVString &response) {
    json j;
    j["result"] = "ok";
    for (auto &kv : response) {
        j[kv.first] = kv.second;
    }
    std::string content = j.dump(2);
    rep = posrep + std::to_string(content.size()) + eol + eol + content;
}

//------------------------------------------------------------------------------
template <typename T>
void AnonymBE<T>::set_negative_response(std::string &rep,
                                        const std::string &msg,
                                        const std::string &extra) {
    json j;
    j["result"] = "error";
    j["detail"] = extra != "" ? extra : msg ;
    
    std::string content = j.dump(2);
    rep = negrep + std::to_string(content.size()) + eol + eol + content;
}

//------------------------------------------------------------------------------
template <typename T>
int AnonymBE<T>::input_file(const std::string &data) {}

//------------------------------------------------------------------------------
template <typename T>
int AnonymBE<T>::init(Arguments *args) {
#ifdef TLS_REQUESTS
    init_openssl( &ctx_ );
#endif
    try {
        database_.init(args->mongo);
        init_ = true;
    } catch(uint32_t e) {
        printf("Error: %lu\n", e);
        return -1;
    }
    return 0;
}

//------------------------------------------------------------------------------
template <typename T>
int AnonymBE<T>::accept(int fd) {
#ifdef TLS_REQUESTS
    return tls_accept( fd, ctx_ );
#endif
}

//------------------------------------------------------------------------------
template <typename T>
void AnonymBE<T>::finish() {}

//------------------------------------------------------------------------------
template <typename T>
bool AnonymBE<T>::printmsg_onerror(int ret, const char *msg) {
    if (ret != SGX_SUCCESS) printf("%s: %s\n", msg, err_msg(ret));
    return ret == SGX_SUCCESS;
}

//------------------------------------------------------------------------------
template <typename T>
std::string AnonymBE<T>::err_msg(int v) {
    switch (v) {
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
template <typename T>
std::string AnonymBE<T>::err_amcs(ASKYError e) {
    switch (e) {
        case ASKY_NOERROR:
            return "No error";
        case ASKY_INITIALIZATION_FAIL:
            return "Fail to initialize AnonymBE";
        case ASKY_ERROR_SERVICE:
            return "Could not access Platform Services";
        case ASKY_BAD_REQUEST:
            return "Malformed request. "
                   "Please, refer to the documentation.";
        case ASKY_CREATE_EXISTENT:
            return "Tried to create an existing user";
        case ASKY_NON_EXISTENT:
            return "Tried to get or update a "
                   "non-existing counter";
        case ASKY_WRAPUP:
            return "Service is being terminated";
        case ASKY_UNKNOWN:
        default:
            return "Unknown error: " + std::to_string(e);
    }
}

//------------------------------------------------------------------------------
