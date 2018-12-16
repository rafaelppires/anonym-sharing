#include <writerproxy_service.h>
#include <tls_config.h>
#include <httpcommon.h>
#include <sgx_cryptoall.h>
#include <digest.h>
#include <json/json.hpp>
using json = nlohmann::json;

//------------------------------------------------------------------------------
WriterProxy::WriterProxy() : minioClient(nullptr) {}
//------------------------------------------------------------------------------
WriterProxy::~WriterProxy() { finish(); }

//------------------------------------------------------------------------------
void WriterProxy::finish() {
    delete minioClient;
    minioClient = nullptr;
    IncomeSSLConnection::finish();
}

//------------------------------------------------------------------------------
int WriterProxy::init(Arguments args) {
    if (strlen(args.minioendpoint) == 0 || strlen(args.minioaccesskey) == 0 ||
        strlen(args.miniosecret) == 0) {
        printf(
            "All minio parameters are mandatory: -m <host:port> -a <ACCESSKEY> "
            "-k <SECRETKEY>\n");
        return -1;
    }

    std::string endpoint(args.minioendpoint);
    if (endpoint.find("http") == 0) {
        printf("Minio endpoint must be specified as <host>:<port>\n");
        return -2;
    }

    args.tokenuser[sizeof(args.tokenuser) - 1] =
        args.tokenpass[sizeof(args.tokenpass) - 1] =
            args.tokenendpoint[sizeof(args.tokenendpoint) - 1] = '\0';
    tuser_ = std::string(args.tokenuser);
    tpass_ = std::string(args.tokenpass);
    std::string tendp(args.tokenendpoint);
    if (tendp.empty() != tuser_.empty() || tendp.empty() != tpass_.empty()) {
        printf(
            "Either none or all 3 Identity Server parameters must be given\n");
        return -3;
    } else if (!tendp.empty()) {
        isendpoint_ = HttpUrl::parse("https://" + tendp);
    }

    IncomeSSLConnection::init();
    header_builder.set(HttpStrings::connection, HttpStrings::keepalive)
        .set("Server", "A-SKY WriterProxy");
    response_builder.protocol(HttpStrings::http11).headers(header_builder);

    std::string minioedp = "https://" + endpoint;
    minioendpoint_ = HttpUrl::parse(minioedp);
    try {
        minioClient =
            new MinioClient(minioedp, args.minioaccesskey, args.miniosecret);
        // minioClient->traceOn(cout);
    } catch (const std::exception &e) {
        printf("%s\n", e.what());
        exit(1);
    }
    return 0;
}

//------------------------------------------------------------------------------
Response WriterProxy::treat_request(const Request &request) {
    if (request.method() == "GET") {
        const HttpUrl &url = request.url();
        if (url.encodedPath() == "/ping") {
            return response_builder.code(200).message("OK").build();
        } else if (url.encodedPath() == "/token") {
            return generate_token(request);
        }
    } else if (request.method() == "PUT") {
        return forward(request);
    }
    return response_builder.code(400).message("Bad Request").build();
}

//------------------------------------------------------------------------------
Response WriterProxy::forward(const Request &request) {
    try {
        minioClient->forwardObject(request);
    } catch (const ErrorResponseException &e) {
        if (e.code() == ErrorCode::NO_SUCH_BUCKET && make_bucket(request)) {
            forward(request);
        } else {
            response_builder.code(409).message("Conflict");
            return response_builder.build();
        }
    } catch (const std::exception &e) {
        // printf("Error: (%s)\n", e.what());
        std::string norep = "No response";
        if (norep == e.what()) {
            response_builder.code(444).message(norep);
            return response_builder.build();
        }
    }
    response_builder.code(200).message("OK");
    return response_builder.build();
}

//------------------------------------------------------------------------------
bool WriterProxy::make_bucket(const Request &request) {
    const HttpUrl &url = request.url();
    std::string path = url.encodedPath(), bucket;
    auto pieces = split(path, "/");
    if (!pieces.empty()) bucket = pieces[0];
    if (bucket.empty()) return false;
    try {
        minioClient->makeBucket(bucket);
    } catch (const std::exception &e) {
        printf("Error creating bucket: %s\n", e.what());
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
std::string WriterProxy::request_reply(const HttpUrl &endpoint,
                                       const Request &request) {
    if (endpoint.port() == -1)
        throw response_builder.code(405).message("Method Not Allowed").build();
    Response error(response_builder.code(504).message("Gateway Timeout"));

    TlsConnection connection;
    if (!connection.connect(endpoint.host(), endpoint.port())) throw error;

    std::string reqstr = request.toString();
    //printf("+%s+\n", reqstr.c_str());
    if (connection.send(reqstr.data(), reqstr.size()) <= 0) throw error;

    Http1Decoder decoder;
    do {
        char buff[5000];
        int len = connection.recv(buff, sizeof(buff));
        if (len < 0) throw error;
        std::string reply(buff, len);
        //printf("(%s)\n", reply.c_str());
        decoder.addChunk(reply);
    } while (!decoder.responseReady());

    Response r = decoder.getResponse();
    if (r.isSuccessful()) {
        return r.body();
    }
    return "";
}

//------------------------------------------------------------------------------
Response WriterProxy::generate_token(const Request &request) {
    RequestBuilder req;
    req.method("POST")
        .url(HttpUrl::parse("http://0:0/oauth2/token"))
        .protocol(HttpStrings::http11)
        .appendBody("grant_type=client_credentials")
        .header("Host",
                isendpoint_.host() + ":" + std::to_string(isendpoint_.port()))
        .header(HttpStrings::auth,
                "Basic " + Digest::base64_encode(tuser_ + ":" + tpass_))
        .header(HttpStrings::contenttype, "application/x-www-form-urlencoded");
    std::string access_token;
    try {
        access_token = request_reply(isendpoint_, req.build());
    } catch (const Response &r) {
        ResponseBuilder rb(r);
        rb.appendBody("Error communicating with Identity Server\n");
        return rb.build();
    }

    auto j = json::parse(access_token);
    UrlBuilder query(minioendpoint_);
    query.addEncodedQueryParameter("Action", "AssumeRoleWithClientGrants")
        .addEncodedQueryParameter("DurationSeconds",
                                  std::to_string(json_int(j, "expires_in")))
        .addEncodedQueryParameter("Token", json_str(j, "access_token"));
    req.clearBody().clearHeaders();
    req.url(query.build())
        .header("Host", minioendpoint_.host() + ":" +
                            std::to_string(minioendpoint_.port()));
    std::string credentials;
    try {
        credentials = request_reply(minioendpoint_, req.build());
    } catch (const Response &r) {
        ResponseBuilder rb(r);
        rb.appendBody("Error communicating with minio\n");
        return rb.build();
    }

    ResponseBuilder rb(response_builder);
    return rb.code(200).message("OK").appendBody(credentials);
}

//------------------------------------------------------------------------------
