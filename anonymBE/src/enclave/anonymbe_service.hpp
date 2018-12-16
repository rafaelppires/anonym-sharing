#include <anonymbe_service.h>
#include <database.h>
#ifndef NATIVE
#include <enclave_anonymbe_t.h>
#include <libc_mock/libcpp_mock.h>
#include <sgx_trts.h>
#endif
#include <sgx_cryptoall.h>
#include <stdio.h>

#define KEY_SIZE 32

template <typename T>
const Response AnonymBE<T>::posrep =
    ResponseBuilder()
        .protocol(HttpStrings::http11)
        .code(200)
        .message("OK")
        .headers(HeadersBuilder()
                     .set(HttpStrings::connection, HttpStrings::keepalive)
                     .set(HttpStrings::contenttype, "application/json"))
        .build();

template <typename T>
const Response AnonymBE<T>::negrep =
    ResponseBuilder()
        .protocol(HttpStrings::http11)
        .code(400)
        .message("Bad Request")
        .headers(HeadersBuilder()
                     .set(HttpStrings::connection, HttpStrings::keepalive)
                     .set(HttpStrings::contenttype, "application/json"))
        .build();

//------------------------------------------------------------------------------
template <typename T>
AnonymBE<T>::AnonymBE()
    : /*database_(true),*/ init_(false), error_(ASKY_NOERROR), die_(false) {
#ifndef NATIVE
    update_mutex_ = SGX_THREAD_MUTEX_INITIALIZER;
    goahead_condition_ = end_condition_ = update_condition_ =
        SGX_THREAD_COND_INITIALIZER;
#endif
}

//------------------------------------------------------------------------------
template <typename T>
typename AnonymBE<T>::ASKYError AnonymBE<T>::process_get(const Request &request,
                                                         KVString &response) {
    const HttpUrl &url = request.url();
    std::string command = url.encodedPath();
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
typename AnonymBE<T>::ASKYError AnonymBE<T>::generate_envelope(
    json &j, KVString &response) {
    std::string ctext;
    std::string bucket_key = Crypto::b64_decode(json_str(j, "bucket_key"));
    bucket_key.resize(32, 0);
    KeyArray ka = database_.get_keys_of_group(json_str(j, "group_id"));

    // if empty
    if (ka.empty()) {
        response["detail"] = "empty envelope";
        return ASKY_BAD_REQUEST;
    }

    if (indexed_) {
        // compute nonce
        std::string nonce = Crypto::get_rand(12);

        // put hashes of <nonce,key> in an ordered container
        KVString orderedhashes;
        for (const auto &k : ka) {
            std::string key((const char *)k.data(), KEY_SIZE);
            orderedhashes[Crypto::sha224(nonce + key)] =
                Crypto::encrypt_aesgcm(key, bucket_key);
        }

        // add hashes and keys in order
        for (const auto &kv : orderedhashes) {
            ctext += kv.first + kv.second;
        }

        // put the nonce in the end
        ctext += nonce;
    } else {
        for (const auto &k : ka) {
            std::string key((const char *)k.data(), KEY_SIZE);
            ctext += Crypto::encrypt_aesgcm(key, bucket_key);
        }
    }
    response["ciphertext"] = Crypto::b64_encode(ctext);
    return ASKY_NOERROR;
}

//------------------------------------------------------------------------------
template <typename T>
typename AnonymBE<T>::ASKYError AnonymBE<T>::create_user(json &j,
                                                         KVString &response) {
    unsigned char rnd[KEY_SIZE];
#ifdef NATIVE
    for (int i = 0; i < sizeof(rnd); i += sizeof(int)) *(int *)&rnd[i] = rand();
#else
    sgx_read_rand(rnd, sizeof(rnd));
#endif
    std::string key = std::string((const char *)rnd, sizeof(rnd));
    database_.create_user(json_str(j, "user_id"), key);
    // if( key == "" ) return ASKY_CREATE_EXISTENT;
    response["user_key"] = Crypto::b64_encode(key);
    return ASKY_NOERROR;
}

//------------------------------------------------------------------------------
template <typename T>
typename AnonymBE<T>::ASKYError AnonymBE<T>::process_put(
    const Request &request) {
    const HttpUrl &url = request.url();
    std::string command = url.encodedPath();
    if (command == "/access/usergroup") {
        auto j = json::parse(request.stringBody());
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
    const Request &request, KVString &response) {
    auto j = json::parse(request.stringBody());
    const HttpUrl &url = request.url();
    std::string command = url.encodedPath();
    if (command == "/access/user") {
        return create_user(j, response);
    } else if (command == "/access/group") {
        database_.create_group(json_str(j, "group_name"),
                               json_str(j, "user_id"));
        return ASKY_NOERROR;
    } else if (command == "/verifier/envelope") {
        return generate_envelope(j, response);
    } else if (command == "/verifier/usergroup") {
        bool answer = database_.is_user_part_of_group(
            json_str(j, "user_id"), json_str(j, "group_name"));
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
    const Request &request) {
    const HttpUrl &url = request.url();
    std::string command = url.encodedPath();
    if (command == "/access/usergroup") {
        auto j = json::parse(request.stringBody());
        database_.remove_user_from_group(json_str(j, "group_name"),
                                         json_str(j, "user_id"));
        return ASKY_NOERROR;
    } else if (command == "/access/user") {
        auto j = json::parse(request.stringBody());
        database_.delete_user(json_str(j, "user_id").c_str());
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
Response AnonymBE<T>::process_input(const Request &request) {
    if (!init_) {
#ifdef MEMDATABASE
        init_ = true;
#else
        printf("Attempt to do some processing before initializing\n");
        return Response();
#endif
    }
    std::string verb, extra;
    ASKYError error = ASKY_NOERROR;
    KVString response;
    verb = request.method();
    try {
        if (verb == "GET") {
            error = process_get(request, response);
        } else if (verb == "PUT") {
            error = process_put(request);
        } else if (verb == "POST") {
            error = process_post(request, response);
        } else if (verb == "DELETE") {
            error = process_delete(request);
        }

        if (error == ASKY_BAD_REQUEST) {
            extra = "Unknown command '" + request.url().encodedPath() + "'";
        }
    } catch (const nlohmann::detail::out_of_range &e) {
        extra = e.what();
        error = ASKY_BAD_REQUEST;
    } catch (const nlohmann::detail::parse_error &e) {
        extra = e.what();
        error = ASKY_BAD_REQUEST;
    } catch (const std::invalid_argument &e) {
        extra = e.what();
        error = ASKY_BAD_REQUEST;
    } catch (const std::logic_error &e) {  // warning, not error
        response["info"] = e.what();
    } catch (uint32_t e) {
        extra = mongo_error(e);
        error = ASKY_UNKNOWN;
    } catch (const std::exception &e) {
        printf("Error: %s\n", e.what());
    } catch (...) {
        error = ASKY_UNKNOWN;
    }

    try {
        if (error == ASKY_NOERROR) {
            return positive_response(response);
        } else {
            return negative_response(err_amcs(error), extra);
        }
    } catch (nlohmann::detail::exception &e) {
        printf("Err: %s\n", e.what());
        for (const auto &kv : response)
            printf("[%s][%s]\n", kv.first.c_str(), kv.second.c_str());
        printf("<%s><%s>\n", err_amcs(error).c_str(), extra.c_str());
    }
    return {};
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
Response AnonymBE<T>::positive_response(const KVString &response) {
    json j;
    j["result"] = "ok";
    for (auto &kv : response) {
        j[kv.first] = kv.second;
    }

    return ResponseBuilder(posrep).appendBody(j.dump(2)).build();
}

//------------------------------------------------------------------------------
template <typename T>
Response AnonymBE<T>::negative_response(const std::string &msg,
                                        const std::string &extra) {
    json j;
    j["result"] = "error";
    j["detail"] = extra != "" ? extra : msg;

    return ResponseBuilder(negrep).appendBody(j.dump(2)).build();
}

//------------------------------------------------------------------------------
template <typename T>
int AnonymBE<T>::input_file(const std::string &data) {}

//------------------------------------------------------------------------------
template <typename T>
int AnonymBE<T>::init(Arguments *args) {
    try {
        database_.init(args->mongo);
        indexed_ = args->indexed != 0;
        init_ = true;
    } catch (uint32_t e) {
        printf("Error: %u\n", e);
        return -1;
    }
    return 0;
}

//------------------------------------------------------------------------------
template <typename T>
void AnonymBE<T>::finish() {}

//------------------------------------------------------------------------------
template <typename T>
bool AnonymBE<T>::printmsg_onerror(int ret, const char *msg) {
#ifdef NATIVE
    return ret == 0;
#else
    if (ret != SGX_SUCCESS) printf("%s: %s\n", msg, sgx_errmsg(ret));
    return ret == SGX_SUCCESS;
#endif
}

//------------------------------------------------------------------------------
template <typename T>
std::string AnonymBE<T>::sgx_errmsg(int v) {
#ifdef NATIVE
    return "";
#else
    switch (v) {
        case SGX_ERROR_AE_SESSION_INVALID:
            return "Session is not created or has been closed "
                   "by architectural enclave service";
        case SGX_ERROR_BUSY:
            return "Service is busy: try later";
        default:
            return "Unknown error code (" + std::to_string(v) + ")";
    };
#endif
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
